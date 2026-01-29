/**
 * CIE Telemetry Generator
 * Simulates vehicle sensor data and sends it over Serial to the ESP-01.
 */

#include <Arduino.h> // Required for PlatformIO

unsigned long packetCount = 0; // Counts how many messages we sent
int fakeSensorValue = 0;       // Simulates a changing sensor (0-100)
int increment = 5;

void setup() {
  // Start Serial communication at 9600 baud
  // This connects to the ESP-01's RX/TX pins
  Serial.begin(9600);
  
  // Wait a moment for the ESP-01 to boot up fully before spamming data
  delay(2000);
}

void loop() {
  // 1. Update the fake sensor data (Oscillate between 0 and 100)
  fakeSensorValue += increment;
  if (fakeSensorValue >= 100 || fakeSensorValue <= 0) {
    increment = -increment; // Reverse direction
  }

  // 2. Increment Packet Counter
  packetCount++;

  // 3. Send the Data to ESP-01
  // The format is: "PacketID | RPM: Value | Bat: Value"
  Serial.print("Packet: ");
  Serial.print(packetCount);
  Serial.print(" | RPM: ");
  Serial.print(fakeSensorValue * 50); // Simulate RPM (0 - 5000)
  Serial.print(" | Bat: ");
  Serial.print(12.0 + (fakeSensorValue / 100.0)); // Simulate Voltage
  Serial.println("V"); // 'println' adds the newline character

  // 4. Wait for 500ms (Send 2 messages per second)
  delay(500);
}