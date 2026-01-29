/**
 * ESP-01 Wi-Fi Bridge
 * Acts as a SoftAP and TCP Server.
 * Relays data between the Serial port (connected to Arduino) and a Wi-Fi Client (Laptop).
 */

#include <Arduino.h>        // Required for PlatformIO
#include <ESP8266WiFi.h>    // Required for Wi-Fi functions

// --- Configuration ---
const char* ssid = "ESP01_Telemetry";
const char* password = "password123"; // Must be at least 8 characters
WiFiServer server(80);                // Port 80 for Telnet/Terminal access

void setup() {
  // 1. Start Serial at 9600 to match the Arduino's baud rate
  Serial.begin(9600);

  // 2. Configure the ESP-01 as an Access Point
  WiFi.softAP(ssid, password);

  // 3. Start the TCP Server
  server.begin();
}

void loop() {
  // Check if a client (your Laptop) has connected via Terminal
  WiFiClient client = server.available();

  if (client) {
    // While the laptop is connected...
    while (client.connected()) {

      // --- DIRECTION 1: ARDUINO TO LAPTOP ---
      // Listen for data coming into the ESP's RX pin from the Arduino
      if (Serial.available() > 0) {
        char fromArduino = Serial.read();
        client.write(fromArduino); // Send it wirelessly to your laptop
      }

      // --- DIRECTION 2: LAPTOP TO ARDUINO ---
      // Listen for data coming from your Laptop wirelessly
      if (client.available() > 0) {
        char fromLaptop = client.read();
        Serial.write(fromLaptop);  // Send it out the ESP's TX pin to the Arduino
      }
      
      // CRITICAL: Feed the Hardware Watchdog Timer (WDT)
      // Without this 'yield', the ESP-01 will crash/reset if data streams too fast.
      yield();