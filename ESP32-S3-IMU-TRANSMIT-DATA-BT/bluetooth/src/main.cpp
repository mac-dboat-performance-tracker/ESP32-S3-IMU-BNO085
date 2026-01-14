#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BNO08x.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- PINS (Matched to your WORKING code) ---
// Note: We use the default SPI bus defined by your board variant.
// If you are on ESP32-S3/Feather, these usually map to SCK=12, MISO=13, MOSI=11
#define BNO08X_CS 10
#define BNO08X_INT 9
#define BNO08X_RESET 5

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

// --- BLE VARIABLES ---
static const char* BLE_DEVICE_NAME = "DB_IMU_BNO085";
static const char* SERVICE_UUID    = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"; // UART Service
static const char* TX_CHAR_UUID    = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; // TX Characteristic

BLECharacteristic* txChar = nullptr;
bool bleConnected = false;

// --- SYNC VARIABLES ---
bool retrieve = true;
float acc_x, acc_y, acc_z;

// --- BLE CALLBACKS ---
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer*) override { bleConnected = true; }
    void onDisconnect(BLEServer*) override { bleConnected = false; }
};

void setReports() {
  Serial.println("Setting desired reports...");
  if (!bno08x.enableReport(SH2_GAME_ROTATION_VECTOR)) {
    Serial.println("Could not enable game vector");
  }
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION)) {
    Serial.println("Could not enable linear acceleration");
  }
}

void setupBLE() {
  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);
  txChar = service->createCharacteristic(TX_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  txChar->addDescriptor(new BLE2902());
  service->start();

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->setScanResponse(true);
  adv->start();
  
  Serial.println("BLE advertising started.");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("BNO085 SPI + BLE (Polling Mode)");

  // Initialize Sensor using the EXACT same method as your working code
  if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) { delay(10); }
  }
  Serial.println("BNO08x Found over SPI!");

  setReports();
  setupBLE();
  
  Serial.println("Waiting for connections...");
}

void loop() {
  // 1. Check for Reset
  if (bno08x.wasReset()) {
    Serial.print("Sensor was reset, re-enabling reports...");
    setReports();
  }

  // 2. Poll for Data (Same logic as your working code)
  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  // 3. Process Data
  switch (sensorValue.sensorId) {
    
    case SH2_LINEAR_ACCELERATION:
      if (retrieve) {
        acc_x = sensorValue.un.linearAcceleration.x;
        acc_y = sensorValue.un.linearAcceleration.y;
        acc_z = sensorValue.un.linearAcceleration.z;
        retrieve = false;   
      }
      break;

    case SH2_GAME_ROTATION_VECTOR:
      if (!retrieve) {
        // Format the line exactly like your working code
        // Order: ax, ay, az, qw, qx, qy, qz
        char line[128];
        
        // Note: Applying your specific mounting adjustment (j -> -i etc)
        // Your map: Real, -j, i, k
        snprintf(line, sizeof(line), 
                 "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", 
                 acc_x, acc_y, acc_z,
                 sensorValue.un.gameRotationVector.real,
                 -sensorValue.un.gameRotationVector.j,
                 sensorValue.un.gameRotationVector.i,
                 sensorValue.un.gameRotationVector.k);

        // Print to Serial
        Serial.println(line);

        // Send to BLE
        if (bleConnected && txChar) {
          txChar->setValue((uint8_t*)line, strlen(line));
          txChar->notify();
        }

        retrieve = true; // Reset sync flag
      }
      break;
  }
}