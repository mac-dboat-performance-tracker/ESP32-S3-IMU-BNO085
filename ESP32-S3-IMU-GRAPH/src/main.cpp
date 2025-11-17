/* Plotter-friendly version of the test sketch for Adafruit BNO08x sensor in
 * UART-RVC mode, formatted for Mario Zechner's VSCode Serial Plotter.
 */

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

  Serial.println("Adafruit BNO08x IMU - UART-RVC mode (Serial Plotter)");

  // Bind Serial1 to the pins
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
    // No fresh data yet, just skip this loop iteration
    return;
  }

  // -------------------------------
  // Format for Mario's Serial Plotter:
  // >name:value,name2:value2,...
  // -------------------------------
  Serial.print(">");

  Serial.print("yaw:");
  Serial.print(heading.yaw);
  Serial.print(",");

  Serial.print("pitch:");
  Serial.print(heading.pitch);
  Serial.print(",");

  Serial.print("roll:");
  Serial.print(heading.roll);
  Serial.print(",");

  Serial.print("x_accel:");
  Serial.print(heading.x_accel);
  Serial.print(",");

  Serial.print("y_accel:");
  Serial.print(heading.y_accel);
  Serial.print(",");

  Serial.print("z_accel:");
  Serial.print(heading.z_accel);

  Serial.println();  // sends \r\n

  // Optional: slow the stream a bit
  // delay(20);
}
