#include <Arduino.h>
#include <SoftwareSerial.h> 

#include "mppt.hpp" 
#include "mppt_config.hpp"
#include "internal_telemetry_packet.hpp"
#include "CUR_SENS.hpp"
#include "CUR_SENS_config.hpp"
#include "VOLT_SENS.hpp"
#include "VOLT_SENS_config.hpp"

// UART Communication with ESP32
#define RX_PIN 2
#define TX_PIN 3
SoftwareSerial espSerial(RX_PIN, TX_PIN); 

internal_payLoad arduinoPayLoad_buffer;

// Telemetry Accumulators
float sum_rpm = 0, sum_power = 0, sum_cConv = 0, sum_vConv = 0, sum_cRect = 0, sum_vRect = 0;
float sum_pitch = 0, sum_temp = 0;
unsigned int samplesTaken = 0;

#define MPPT_PWM_PIN 6

// =================================================================
// HARDWARE INSTANTIATIONS (Exact requested Pinout & Signatures)
// =================================================================

// Voltage 1: Rectifier -> Pin A2 (Single ended, uses pre-calculated 47k/2.2k ratio)
VoltageSensor rectifierVoltage(A2, VOLTAGE_DIVIDER_RATIO);

// Voltage 2: Converter -> Pins A3 & A4 (Differential, multiplied by 250)
VoltageSensor converterVoltage(A3, A4, 125.0f);

// Current 1: Rectifier -> Pin A0 (5V Ref, 100mV/A 20A sensitivity)
CurrentSensor rectifierCurrent(A0, 5.0f, ACS712_20_SENSITIVITY);

// Current 2: Converter -> Pin A1 (5V Ref, 100mV/A 20A sensitivity)
CurrentSensor converterCurrent(A1, 5.0f, ACS712_20_SENSITIVITY);


// TIMING CLOCKS
unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 20;  

unsigned long lastReportTime = 0;
const unsigned long reportInterval = 500; 

// Instantaneous Live Values
float rpm = 0, power = 0, currentConv = 0, voltageConv = 0, currentRect = 0, voltageRect = 0;
float pitchAngle = 0, temperature = 0;
float dutyCycle = 0.5f; 

void setup() {
  Serial.begin(9600); 
  espSerial.begin(9600); 

  pinMode(MPPT_PWM_PIN, OUTPUT);

  // 1. Initialize hardware pins
  converterCurrent.begin();
  rectifierCurrent.begin();
  converterVoltage.begin();
  rectifierVoltage.begin();

  // 2. CRITICAL: Trigger the blocking zero-current calibration!
  // (Ensure turbine generator is stationary / open-circuit during this 2-second window)
  Serial.println("Calibrating Rectifier Hall Sensor (A0)...");
  rectifierCurrent.calibrateCurrentSensor();
  
  Serial.println("Calibrating Converter Hall Sensor (A1)...");
  converterCurrent.calibrateCurrentSensor();

  // Initialize buffer payload defaults
  arduinoPayLoad_buffer.syncByte_1  = 0xAA;
  arduinoPayLoad_buffer.syncByte_2  = 0xBB;
  arduinoPayLoad_buffer.brakeACK    = 0; 
  arduinoPayLoad_buffer.rpm         = 0;
  arduinoPayLoad_buffer.power       = 0;
  arduinoPayLoad_buffer.currentConv = 0;
  arduinoPayLoad_buffer.voltageConv = 0;
  arduinoPayLoad_buffer.currentRect = 0;
  arduinoPayLoad_buffer.voltageRect = 0;
  arduinoPayLoad_buffer.pitchAngle  = 0;
  arduinoPayLoad_buffer.temperature = 0;

  Serial.println("EurUs Power Architecture Online. Dispatching UART telemetry...");
}

void loop() {
  
  // --- 0. LISTEN FOR ESP32 EMERGENCY BRAKE ---
  if (espSerial.available() > 0) {         
    char cmd = espSerial.read();           
    if (cmd == 'S') { 
      arduinoPayLoad_buffer.brakeACK = 1;   
      Serial.println(">>> E-STOP COMMAND VERIFIED FROM ESP32 <<<");           
    } 
  }

  // --- 1. HIGH-SPEED SENSOR SAMPLING (50Hz / 20ms) ---
  if (millis() - lastSampleTime >= sampleInterval) {
    lastSampleTime = millis();

    // A. Sample live currents
    currentRect = rectifierCurrent.readCurrent();
    currentConv = converterCurrent.readCurrent(); 

    // B. Sample live voltages 
    voltageRect = rectifierVoltage.readVoltage();
    voltageConv = converterVoltage.readVoltage();

    // C. True instantaneous converter power
    power = voltageConv * currentConv;

    // Bucket accumulation
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


  // --- 2. TELEMETRY DISPATCH & MPPT EXECUTION (2Hz / 500ms) ---
  if (millis() - lastReportTime >= reportInterval) {
    lastReportTime = millis();

    if (samplesTaken > 0) {
      
      // Compute arithmetic means over the sample window
      arduinoPayLoad_buffer.rpm         = sum_rpm / samplesTaken;
      arduinoPayLoad_buffer.power       = sum_power / samplesTaken;
      arduinoPayLoad_buffer.currentConv = sum_cConv / samplesTaken;
      arduinoPayLoad_buffer.voltageConv = sum_vConv / samplesTaken;
      arduinoPayLoad_buffer.currentRect = sum_cRect / samplesTaken;
      arduinoPayLoad_buffer.voltageRect = sum_vRect / samplesTaken;
      arduinoPayLoad_buffer.pitchAngle  = sum_pitch / samplesTaken;
      arduinoPayLoad_buffer.temperature = sum_temp / samplesTaken;

      // Solve MPPT step using stable averaged inputs
      dutyCycle = MPPT(arduinoPayLoad_buffer.voltageConv, arduinoPayLoad_buffer.currentConv);
      
      // Drive Gate
      analogWrite(MPPT_PWM_PIN, (int)(dutyCycle * 255));

      // Push raw binary struct to ESP32
      espSerial.write((uint8_t*)&arduinoPayLoad_buffer, sizeof(internal_payLoad)); 


      Serial.println("┌─── EURUS POWER BUS (500ms Averaged Frame) ───┐");
        
        Serial.print("│ Rectifier In  : "); 
        Serial.print(arduinoPayLoad_buffer.voltageRect, 2); Serial.print(" V   @   "); 
        Serial.print(arduinoPayLoad_buffer.currentRect, 2); Serial.println(" A");
        
        Serial.print("│ Converter Out : "); 
        Serial.print(arduinoPayLoad_buffer.voltageConv, 2); Serial.print(" V   @   "); 
        Serial.print(arduinoPayLoad_buffer.currentConv, 2); Serial.println(" A");
        
        Serial.print("│ Turbine Power : "); 
        Serial.print(arduinoPayLoad_buffer.power, 2); Serial.println(" W");

        Serial.print("│ Rotor Speed   : "); 
        Serial.print(arduinoPayLoad_buffer.rpm); Serial.println(" RPM");

        Serial.print("│ Pitch / Temp  : "); 
        Serial.print(arduinoPayLoad_buffer.pitchAngle, 1); Serial.print(" °   /   "); 
        Serial.print(arduinoPayLoad_buffer.temperature, 1); Serial.println(" °C");

        Serial.print("│ MPPT Drive    : "); 
        Serial.print(dutyCycle * 100.0f, 1); Serial.print(" %   (PWM Gate: "); 
        Serial.print((int)(dutyCycle * 255)); Serial.println(")");

        Serial.print("│ ESP32 E-Stop  : "); 
        Serial.println(arduinoPayLoad_buffer.brakeACK ? "[[ ENGAGED - BRAKING ]]" : "CLEAR [OK]");
        
        Serial.println("└──────────────────────────────────────────────┘");
        Serial.println(); // Blank line spacing


      // Reset buckets
      sum_rpm = 0; sum_power = 0; sum_cConv = 0; sum_vConv = 0; 
      sum_cRect = 0; sum_vRect = 0; sum_pitch = 0; sum_temp = 0;
      samplesTaken = 0;


    }
  }
}