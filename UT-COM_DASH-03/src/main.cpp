#include <Arduino.h>
#include "BluetoothSerial.h"

// --- GROUP A PARAMETERS (2 Hz Master) ---
float rpm         = 500;
float power       = 800; // Watts
float currentConv = 12.5;  
float voltageConv = 24.0;  
float currentRect = 16.5;  
float voltageRect = 19.5;  

// --- GROUP B PARAMETER (1 Hz) ---
float pitchAngle  = 12.5;

// --- GROUP C PARAMETER (1/60 Hz) ---
float temperature = 42.0;

// Command signals 
bool stopRequested = false;
bool stopAck = false;

unsigned long lastMasterTick = 0;
const unsigned long masterInterval = 500;
unsigned int tickCounter = 0;

BluetoothSerial SerialBT;


//Sensor data generators by group
void triggerSensorA() { 
  rpm = 310.0 + random(-10, 11) * 0.1; 

  
  voltageRect = 19.5 + random(-5, 6) * 0.1; 
  currentRect = 16.5 + random(-4, 5) * 0.1; 

  
  voltageConv = 24.0 + random(-2, 3) * 0.1; 
  //Trying to simulate some converter output
  float inputPower = voltageRect * currentRect;
  currentConv = (inputPower * 0.93) / voltageConv;

  power = voltageConv * currentConv; 
}
void triggerSensorB() { pitchAngle = 12.5 + random(-5, 6) * 0.1; }
void triggerSensorC() { temperature = 42.0 + random(-2, 3) * 0.1; }

// PayLoad generator
String generate_payLoad(char statusA, char statusB, char statusC, 
                        float rpm, float pwr, float cConv, float vConv, float cRect, float vRect, 
                        float pitch, float temp, bool isAck) 
{
  return String("$EURUS") + "," + 
         statusA + "," + statusB + "," + statusC + "," + 
         String(rpm, 1)   + "," + 
         String(pwr, 1)   + "," + 
         String(cConv, 2) + "," + 
         String(vConv, 2) + "," + 
         String(cRect, 2) + "," + 
         String(vRect, 2) + "," + 
         String(pitch, 1) + "," + 
         String(temp, 1)  + "," + 
         String(isAck ? 1 : 0);
}

void setup() {
  Serial.begin(9600); 
  SerialBT.begin("EURUS_COMHUB_ESP32");
  Serial.println("Eurus Telemetry Bridge Online.");
}

void loop() {

  if (SerialBT.available()) {
    String incoming = SerialBT.readStringUntil('\n');
    incoming.trim();

    if (incoming == "S") {
      stopRequested = true;
      stopAck = true;
      Serial.println("SAFETY: MANUAL STOP TRIGGERED");
    }
    else if (incoming == "R") {
      stopRequested = false;
      stopAck = false;
      Serial.println("SAFETY: RESET SENT, Brakes disengaged");
    }
  }

  if (millis() - lastMasterTick >= masterInterval) {
    lastMasterTick = millis();
    tickCounter++;

    // Group A (2 Hz)
    char statusA = 'A'; 
    triggerSensorA();

    // Group B (1 Hz)
    char statusB = 'F'; 
    if (tickCounter % 2 == 0) {
      statusB = 'B'; 
      triggerSensorB();
    }

    // Group C (1/60 Hz)
    char statusC = 'F'; 
    if (tickCounter % 120 == 0) {
      statusC = 'C'; 
      tickCounter = 0; 
      triggerSensorC();
    }

    String payLoad = generate_payLoad(statusA, statusB, statusC, 
                                      rpm, power, currentConv, voltageConv, currentRect, voltageRect, 
                                      pitchAngle, temperature, stopAck);

    SerialBT.println(payLoad);
  }
}