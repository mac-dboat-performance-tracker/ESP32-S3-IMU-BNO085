#include "Adafruit_BNO08x_RVC.h"

Adafruit_BNO08x_RVC rvc = Adafruit_BNO08x_RVC();

// UART1 RX = 18, TX = 17 (TX is unused in RVC mode)
static const int RVC_RX_PIN = 18;
static const int RVC_TX_PIN = 17;  // not actually wired

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("Adafruit BNO08x IMU - UART-RVC mode");

  // bind Serial1 to the pins
  Serial1.begin(115200, SERIAL_8N1, RVC_RX_PIN, RVC_TX_PIN);

  if (!rvc.begin(&Serial1)) {
    Serial.println("Could not find BNO08x!");
    while (1) {
      delay(10);
    }
  }

  Serial.println("BNO08x found!");
}

void loop() {
  BNO08x_RVC_Data heading;

  if (!rvc.read(&heading)) {
    Serial.println("No RVC data yet...");
    return;
  }

  Serial.println();
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Principal Axes:"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("Yaw: "));
  Serial.print(heading.yaw);
  Serial.print(F("\tPitch: "));
  Serial.print(heading.pitch);
  Serial.print(F("\tRoll: "));
  Serial.println(heading.roll);
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Acceleration"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("X: "));
  Serial.print(heading.x_accel);
  Serial.print(F("\tY: "));
  Serial.print(heading.y_accel);
  Serial.print(F("\tZ: "));
  Serial.println(heading.z_accel);
  Serial.println(F("---------------------------------------"));


  //  delay(200);
}