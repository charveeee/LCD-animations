#include <Wire.h>

void setup() {
  Wire.begin(21, 22); 
  Serial.begin(115200);
  delay(1000);
  Serial.println("Scanning I2C bus...");
}

void loop() {
  byte count = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Found device at address 0x");
      Serial.println(address, HEX);
      count++;
    }
  }
  if (count == 0) {
    Serial.println("No I2C devices found. Check wiring.");
  }
  delay(3000);
}
