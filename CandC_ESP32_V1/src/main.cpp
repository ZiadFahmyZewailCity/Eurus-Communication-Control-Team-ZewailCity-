#include <Arduino.h>
#include "BluetoothSerial.h"
#include "../include/internal_telemetry_packet.hpp"

// --- HARDWARE UART DEFINITIONS ---
#define RXD2 16
#define TXD2 17

// --- TELEMETRY PARAMETERS ---
float rpm = 0;     
float power = 0;
float currentConv = 0;  
float voltageConv = 0;  
float currentRect = 0;  
float voltageRect = 0;
float pitchAngle = 0;
float temperature = 0;

// Command signals 
bool stopRequested = false;
uint8_t stopAck = 0;

// Tracking sending of data at given intervals
unsigned long lastMasterTick = 0;
const unsigned long masterInterval = 500; // 2Hz
unsigned int tickCounter = 0;

// Object for managing serial communication
BluetoothSerial SerialBT;
// Arduino payload buffer 
internal_payLoad arduinoPayload_buffer;

void setup() {
  Serial.begin(9600); 
  // Hardware UART to Arduino
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  
  
  // Start Bluetooth Server
  SerialBT.begin("fixTemp");
  Serial.println("Eurus Telemetry Bridge Online (Bluetooth Only).");
}

void loop() {

  // ------------------------------------------------------------------
  // 1. INCOMING UART PIPELINE (From Arduino)
  // ------------------------------------------------------------------
  if(Serial2.available() >= sizeof(internal_payLoad)) {

    if(Serial2.peek() == 0xAA) {
      // Read the full packet into the struct
      Serial2.readBytes((uint8_t*)&arduinoPayload_buffer, sizeof(internal_payLoad));

      // Verify the tail sync byte
      if(arduinoPayload_buffer.syncByte_2 == 0xBB) {
        stopAck     = arduinoPayload_buffer.brakeACK;
        rpm         = arduinoPayload_buffer.rpm;
        power       = arduinoPayload_buffer.power;
        currentConv = arduinoPayload_buffer.currentConv;
        voltageConv = arduinoPayload_buffer.voltageConv;
        currentRect = arduinoPayload_buffer.currentRect;
        voltageRect = arduinoPayload_buffer.voltageRect;
        pitchAngle  = arduinoPayload_buffer.pitchAngle;
        temperature = arduinoPayload_buffer.temperature;
        
        // Uncomment for local debugging
        // Serial.println("Valid packet received from Arduino.");
      } else {
        // Corrupted payload tail - flush buffer to allow clean resync
        while(Serial2.available() > 0) {
            Serial2.read();
        }
      }
    } else {
      // Skip this malformed byte to re-align the buffer header
      Serial2.read();
    }
  }

  // ------------------------------------------------------------------
  // 2. HANDLING COMMAND SIGNALS FROM DASHBOARD (BLUETOOTH)
  // ------------------------------------------------------------------
  if (SerialBT.available()) {
    String incoming = SerialBT.readStringUntil('\n');
    incoming.trim();

    if (incoming == "S") {
      stopRequested = true;
      Serial2.write('S');
      Serial.println("Stop command received via BT, forwarding to Arduino.");
    }
  }

  // ------------------------------------------------------------------
  // 3. REAL-TIME DISPATCH: MATLAB/BLUETOOTH DASHBOARD (500ms / 2Hz)
  // ------------------------------------------------------------------
  if (millis() - lastMasterTick >= masterInterval) {
    lastMasterTick = millis();
    tickCounter++;

    // Group A (2 Hz)
    char statusA = 'A'; 

    // Group B (1 Hz)
    char statusB = 'F'; 
    if (tickCounter % 2 == 0) {
      statusB = 'B'; 
    }

    // Group C (1/60 Hz)
    char statusC = 'F'; 
    if (tickCounter % 120 == 0) {
      statusC = 'C'; 
      tickCounter = 0; 
    }

    // Generate payload using a C-string buffer for memory stability
    char btPayload[128];
    snprintf(btPayload, sizeof(btPayload), "$EURUS,%c,%c,%c,%.1f,%.1f,%.2f,%.2f,%.2f,%.2f,%.1f,%.1f,%d",
             statusA, statusB, statusC, 
             rpm, power, currentConv, voltageConv, currentRect, voltageRect, 
             pitchAngle, temperature, stopAck);

    // Blast out to connected Bluetooth device
    SerialBT.println(btPayload);
  }
}