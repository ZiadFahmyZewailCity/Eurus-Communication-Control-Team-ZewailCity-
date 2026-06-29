#pragma once
#include <Arduino.h>

class VoltageSensor {
  private:
    uint8_t pinPos;
    uint8_t pinNeg;
    bool isDifferential;
    float scaleFactor;

  public:
    // Personality 1: Single-Ended (Rectifier on A2)
    VoltageSensor(uint8_t pin, float multiplier);

    // Personality 2: Differential (Converter on A3 & A4)
    VoltageSensor(uint8_t posPin, uint8_t negPin, float multiplier);

    void begin();
    float readVoltage();
};