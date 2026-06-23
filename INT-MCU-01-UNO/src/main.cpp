#include <Arduino.h>
#include <SoftwareSerial.h> // <-- 1. ADDED NATIVE LIBRARY
#include "internal_telemetry_packet.hpp" 

// --- DECOUPLED ESP32 SERIAL PORT ---
// Arduino D2 (RX) <---> Connects to ESP32 TX
// Arduino D3 (TX) <---> Connects to ESP32 RX
SoftwareSerial espSerial(2, 3); // <-- 2. DEFINED D2/D3

// --- Raw instantaneous values ---
float rpm, power, currentConv, voltageConv, currentRect, voltageRect;
float pitchAngle;
float temperature;

// --- SUM BUCKETS FOR AVERAGING ---
float sum_rpm = 0, sum_power = 0, sum_cConv = 0, sum_vConv = 0, sum_cRect = 0, sum_vRect = 0;
float sum_pitch = 0;
float sum_temp = 0;
unsigned int samplesTaken = 0;

// --- DECOUPLED CLOCKS ---
unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 20;  

unsigned long lastReportTime = 0;
const unsigned long reportInterval = 500; 

internal_payLoad arduinoPayLoad_buffer;

// =================================================================
// SIMULATED SENSORS (100% Unchanged)
// =================================================================
void triggerSensorA() { 
  rpm = 310.0 + random(-10, 11) * 0.1; 
  voltageRect = 19.5 + random(-5, 6) * 0.1; 
  currentRect = 16.5 + random(-4, 5) * 0.1; 
  voltageConv = 24.0 + random(-2, 3) * 0.1; 
  
  float inputPower = voltageRect * currentRect;
  currentConv = (inputPower * 0.93) / voltageConv;
  power = voltageConv * currentConv; 
}
void triggerSensorB() { pitchAngle = 12.5 + random(-5, 6) * 0.1; }
void triggerSensorC() { temperature = 42.0 + random(-2, 3) * 0.1; }

// =================================================================

void setup() {
  // 3. Standard Serial is now strictly reserved for your PC screen
  Serial.begin(9600); 

  // 4. Open the Software line to the ESP32
  espSerial.begin(9600); 

  arduinoPayLoad_buffer.syncByte_1 = 0xAA;
  arduinoPayLoad_buffer.syncByte_2 = 0xBB;
  arduinoPayLoad_buffer.brakeACK = 0; 

  Serial.println("Arduino booted. Broadcasting to ESP on D2/D3...");
}

void loop() {

  // 1. CHECK FOR EMERGENCY STOP (Listening to D2)
  if (espSerial.available() > 0) {         // <-- Changed Serial to espSerial
    char cmd = espSerial.read();           // <-- Changed Serial to espSerial
    if (cmd == 'S') { 
      arduinoPayLoad_buffer.brakeACK = 1;   
      Serial.println(">>> E-STOP FLAG RECEIVED FROM ESP32 <<<");           
    } 
  }

  // =================================================================
  // CLOCK 1: THE FAST SAMPLER (Unchanged)
  // =================================================================
  if (millis() - lastSampleTime >= sampleInterval) {
    lastSampleTime = millis();

    triggerSensorA();
    triggerSensorB();
    triggerSensorC();

    sum_rpm   += rpm;
    sum_power += power;
    sum_cConv += currentConv;
    sum_vConv += voltageConv;
    sum_cRect += currentRect;
    sum_vRect += voltageRect;
    sum_pitch += pitchAngle;
    sum_temp  += temperature;

    samplesTaken++; 
  }

  // =================================================================
  // CLOCK 2: THE ESP32 REPORTER (Every 500ms)
  // =================================================================
  if (millis() - lastReportTime >= reportInterval) {
    lastReportTime = millis();

    if (samplesTaken > 0) {
      
      arduinoPayLoad_buffer.rpm         = sum_rpm / samplesTaken;
      arduinoPayLoad_buffer.power       = sum_power / samplesTaken;
      arduinoPayLoad_buffer.currentConv = sum_cConv / samplesTaken;
      arduinoPayLoad_buffer.voltageConv = sum_vConv / samplesTaken;
      arduinoPayLoad_buffer.currentRect = sum_cRect / samplesTaken;
      arduinoPayLoad_buffer.voltageRect = sum_vRect / samplesTaken;
      arduinoPayLoad_buffer.pitchAngle  = sum_pitch / samplesTaken;
      arduinoPayLoad_buffer.temperature = sum_temp / samplesTaken;

      // Blast the binary struct out of Pin 3 to the ESP32
      espSerial.write((uint8_t*)&arduinoPayLoad_buffer, sizeof(internal_payLoad)); // <-- Changed Serial to espSerial

      // Print a friendly human confirmation to your computer monitor
      Serial.print("Dispatched packet. Sample count used: ");
      Serial.println(samplesTaken);

      sum_rpm = 0; sum_power = 0; sum_cConv = 0; sum_vConv = 0; 
      sum_cRect = 0; sum_vRect = 0; sum_pitch = 0; sum_temp = 0;
      samplesTaken = 0;
    }
  }
}