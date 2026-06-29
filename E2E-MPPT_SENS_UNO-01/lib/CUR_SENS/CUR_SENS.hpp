#pragma once
#include <Arduino.h>

class CurrentSensor {
  private:
    // These variables belong to the specific sensor instance
    uint8_t pin;
    float vref;
    float sensitivity; 
    
    // Internal state variables
    int zeroPoint_ADCVAL;
    float filtered_current;
    float alpha; // EMA filter weight

  public:
    // Constructor
    CurrentSensor(uint8_t analogPin, float referenceVoltage, float sensorSensitivity);

    // Initialization and Calibration
    void begin();
    bool calibrateCurrentSensor();

    // The function you'll call in your main loop
    float readCurrent();
};