/*
 * BNO08x UART-RVC + WiFi UDP sender (ESP32-S3)
 *
 * - Reads IMU data via UART-RVC on Serial1
 * - Prints data to Serial in Mario Zechner Serial Plotter format
 * - Sends the same line over WiFi via UDP to a PC
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include "Adafruit_BNO08x_RVC.h"

Adafruit_BNO08x_RVC rvc;

// ---------- WiFi CONFIG ----------
// TODO: put your WiFi name/password here
const char* WIFI_SSID     = "BELL614-2.4";
const char* WIFI_PASSWORD = "zaza";

// TODO: put your laptop's IP here (from ipconfig / ifconfig)
IPAddress UDP_HOST(192, 168, 2, 85);  // e.g. 192.168.2.85 -> UDP_HOST(192, 168, 2, 85)
const uint16_t UDP_PORT = 5005;        // must match Python script

WiFiUDP udp;

// UART1 pins on ESP32-S3
static const int RVC_RX_PIN = 18;  // connected to BNO085 SDA (RVC TX)
static const int RVC_TX_PIN = 17;  // unused in RVC mode, but must be defined

void setup() {
  // USB serial for debug + plotting
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println();
  Serial.println("BNO08x IMU - UART-RVC + WiFi UDP");

  // ---------- CONNECT TO WiFi ----------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  // Bind a local UDP port (0 = pick any available)
  udp.begin(0);

  // ---------- SETUP UART FOR RVC ----------
  Serial1.begin(115200, SERIAL_8N1, RVC_RX_PIN, RVC_TX_PIN);

  if (!rvc.begin(&Serial1)) {
    Serial.println("Could not find BNO08x!");
    while (1) {
      delay(10);
    }
  }

  Serial.println("BNO08x found!");
  Serial.println("Streaming IMU data over Serial and UDP...");
}

void loop() {
  BNO08x_RVC_Data heading;

  // Try to read a packet from the IMU
  if (!rvc.read(&heading)) {
    // No new data yet; just skip this iteration
    return;
  }

  // Build the data line in Serial Plotter format:
  // >yaw:...,pitch:...,roll:...,x_accel:...,y_accel:...,z_accel:...
  String line = ">";
  line += "yaw:";      line += heading.yaw;
  line += ",pitch:";   line += heading.pitch;
  line += ",roll:";    line += heading.roll;
  line += ",x_accel:"; line += heading.x_accel;
  line += ",y_accel:"; line += heading.y_accel;
  line += ",z_accel:"; line += heading.z_accel;

  // --------- 1) Print to USB Serial (for Serial Monitor / Serial Plotter) ---------
  Serial.println(line);

  // --------- 2) Send over UDP to your laptop ---------
  // Use the IPAddress overload (no DNS)
  if (udp.beginPacket(UDP_HOST, UDP_PORT) == 0) {
    // If you want, you can debug this:
    // Serial.println("UDP beginPacket failed");
    return;
  }

  udp.print(line);
  udp.endPacket();
}
  // Optional: slow the
