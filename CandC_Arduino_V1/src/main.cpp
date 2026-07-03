#include <Arduino.h>

#include "../include/pinDefinitions.hpp"
#include "../include/internal_telemetry_packet.hpp"

#include "MPPT_CONTROL.hpp"
#include "MPPT_CONTROL_config.hpp"
#include "ACS12_SENSOR.hpp"
#include "ACS12_SENSOR_config.hpp"
#include "POTENTIAL_DIVIDER.hpp"
#include "CONVERTER_DIFFERENTIAL.hpp"
#include "RPM_ENCODER.hpp"
#include "TEMP_SENSOR.hpp"
#include "TEMP_SENSOR_config.hpp"
#include "DC_MOTOR_BREAKING.hpp"


#define espSerial Serial2

internal_payLoad arduinoPayLoad_buffer;

// Telemetry Accumulators
float sum_rpm = 0, sum_power = 0, sum_cConv = 0, sum_vConv = 0, sum_cRect = 0, sum_vRect = 0;
float sum_pitch = 0, sum_temp = 0;
unsigned int samplesTaken = 0;

// TIMING CLOCKS
unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 20;

unsigned long lastReportTime = 0;
const unsigned long reportInterval = 500;

// Instantaneous Live Values
float rpm = 0, power = 0, currentConv = 0, voltageConv = 0, currentRect = 0, voltageRect = 0, pitchAngle = 0, temperature = 0, dutyCycle = 0.5f; 

//Brakes intiall
bool brakeEngaged = false;

void setup() {
  //For debuggin, connect via USB
  Serial.begin(9600);       
  espSerial.begin(9600);    

  //Intialization
  currentSensor_intialize(SENSOR_1);
  currentSensor_intialize(SENSOR_2);
  voltage_PD_configure();
  differentialVoltage_config();
  RPM_ENCODER_config();
  init_tempSensor(TEMP_SENSOR_1);
  init_tempSensor(TEMP_SENSOR_2);
  configureMPPT();
  configureBrakeSystem();   
                            
  //This is the calibration of the current sensors
  Serial.println("Calibrating Rectifier Hall Sensor (A0)...");
  currentSensor_calibrate_blocking(SENSOR_1);

  //This is the calibration of the current sensors
  Serial.println("Calibrating Converter Hall Sensor (A1)...");
  currentSensor_calibrate_blocking(SENSOR_2);

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

  Serial.println("Intialization complete");
}

void loop() {

  // --- 0. LISTEN FOR ESP32 EMERGENCY BRAKE ---
  if (espSerial.available() > 0) {
    char cmd = espSerial.read();
    if (cmd == 'S' && !brakeEngaged) {
      arduinoPayLoad_buffer.brakeACK = 1;
      brakeEngaged = true;   // latched: braking loop now runs every pass below,
                              // no release/disengage path is exposed by the module
      Serial.println(">>> E-STOP COMMAND VERIFIED FROM ESP32 — BRAKE ENGAGED <<<");
    }
  }

  // --- 1. BRAKE CONTROL LOOP ---
  // Runs every pass once engaged. updateBrakingLoop() self-paces internally
  // to samplingTime (10ms), so calling it every loop() pass is safe and correct.
  if (brakeEngaged) {
    updateBrakingLoop();
  }

  // --- 2. HIGH-SPEED SENSOR SAMPLING (50Hz / 20ms) ---
  if (millis() - lastSampleTime >= sampleInterval) {
    lastSampleTime = millis();

    //Current measurmnets
    currentRect = -currentSensor_measurment(SENSOR_1);
    currentConv = currentSensor_measurment(SENSOR_2);

    //Voltage Measurments
    voltageRect = voltage_PD_MeasureValue();
    voltageConv = -differentialVoltage_measurment();

    //RPM Speed
    rpm = RPM_ENCODER_getValue();

    //Instantanous power 
    power = voltageConv * currentConv;

    //Temperature readings
    tempSensor_request(TEMP_SENSOR_1);
    tempSensor_request(TEMP_SENSOR_2);
    float temp1 = tempSensor_measurment(TEMP_SENSOR_1);
    float temp2 = tempSensor_measurment(TEMP_SENSOR_2);
    //Averaged Temperature
    temperature = (temp1 + temp2) * 0.5f;

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


  //Telemetry
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

      //MPPT - Decimated to run every 5th frame (2.5 seconds) for mechanical inertia
      static uint8_t mpptTick = 0;
      static uint16_t timerGateValue = 0; // Statically holds the last value for telemetry prints
      mpptTick++;

      if (mpptTick >= 5) {
          dutyCycle = MPPT(arduinoPayLoad_buffer.voltageRect, arduinoPayLoad_buffer.currentRect);
          timerGateValue = computeTimerOCR(dutyCycle);
          applyPWM(timerGateValue);
          mpptTick = 0; // Reset counter
      }

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
        Serial.print(dutyCycle * 100.0f, 1); Serial.print(" %   (Timer Gate: ");
        Serial.print(timerGateValue); Serial.print(" / "); Serial.print(MPPT_TIMER_TOP); Serial.println(")");

        Serial.print("│ Brake Status  : ");
        if (!brakeEngaged) {
          Serial.println("CLEAR [OK]");
        } else {
          Serial.print("[[ ENGAGED ]]  pos=");
          // Note: ensure getPosition() and getError() are correctly imported in main.cpp
          Serial.print(getPosition(), 3);
          Serial.print(" rad  err=");
          Serial.print(getError(), 3);
          Serial.println(" rad");
        }

        Serial.println("└──────────────────────────────────────────────┘");
        Serial.println(); // Blank line spacing


      // Reset buckets
      sum_rpm = 0; sum_power = 0; sum_cConv = 0; sum_vConv = 0;
      sum_cRect = 0; sum_vRect = 0; sum_pitch = 0; sum_temp = 0;
      samplesTaken = 0;


    }
  }
}