#include <Arduino.h>
#include "internal_telemetry_packet.hpp" 

//NEED TO REVIEW CODE, TOO MANY CHANGES BY AI, PROBABLY NO LONGER UNDERSTAND CODE


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
const unsigned long sampleInterval = 20;  // 50 Hz Oversampling

unsigned long lastReportTime = 0;
const unsigned long reportInterval = 500; // 2 Hz ESP32 broadcast

internal_payLoad arduinoPayLoad_buffer;


// =================================================================
// SIMULATED SENSORS (Unchanged)
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
  Serial.begin(9600); // To ESP32

  arduinoPayLoad_buffer.syncByte_1 = 0xAA;
  arduinoPayLoad_buffer.syncByte_2 = 0xBB;
  arduinoPayLoad_buffer.brakeACK = 0; 
}

void loop() {

  // 1. CHECK FOR EMERGENCY STOP
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'S') { 
      arduinoPayLoad_buffer.brakeACK = 1;              
    } 
  }

  // =================================================================
  // CLOCK 1: THE FAST SAMPLER (Every 20ms)
  // =================================================================
  if (millis() - lastSampleTime >= sampleInterval) {
    lastSampleTime = millis();

    // 1. Fire the sensors
    triggerSensorA();
    triggerSensorB();
    triggerSensorC();

    // 2. Dump the new readings into the sum buckets
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

    // Safety check: Prevent a divide-by-zero crash if the loop lagged
    if (samplesTaken > 0) {
      
      // Calculate the averages and commit them straight to the out-buffer
      arduinoPayLoad_buffer.rpm         = sum_rpm / samplesTaken;
      arduinoPayLoad_buffer.power       = sum_power / samplesTaken;
      arduinoPayLoad_buffer.currentConv = sum_cConv / samplesTaken;
      arduinoPayLoad_buffer.voltageConv = sum_vConv / samplesTaken;
      arduinoPayLoad_buffer.currentRect = sum_cRect / samplesTaken;
      arduinoPayLoad_buffer.voltageRect = sum_vRect / samplesTaken;
      arduinoPayLoad_buffer.pitchAngle  = sum_pitch / samplesTaken;
      arduinoPayLoad_buffer.temperature = sum_temp / samplesTaken;

      // Blast the locked struct to the ESP32
      Serial.write((uint8_t*)&arduinoPayLoad_buffer, sizeof(internal_payLoad));

      // EMPTY THE BUCKETS FOR THE NEXT 500ms WINDOW
      sum_rpm = 0; sum_power = 0; sum_cConv = 0; sum_vConv = 0; 
      sum_cRect = 0; sum_vRect = 0; sum_pitch = 0; sum_temp = 0;
      
      samplesTaken = 0;
    }
  }
}