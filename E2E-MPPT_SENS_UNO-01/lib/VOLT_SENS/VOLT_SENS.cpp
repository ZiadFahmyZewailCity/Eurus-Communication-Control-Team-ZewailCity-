#include "VOLT_SENS.hpp"

// Constructor 1: Single Ended
VoltageSensor::VoltageSensor(uint8_t pin, float multiplier) {
    pinPos = pin;
    pinNeg = 255; 
    isDifferential = false;
    scaleFactor = multiplier;
}

// Constructor 2: Differential
VoltageSensor::VoltageSensor(uint8_t posPin, uint8_t negPin, float multiplier) {
    pinPos = posPin;
    pinNeg = negPin;
    isDifferential = true;
    scaleFactor = multiplier;
}

void VoltageSensor::begin() {
    pinMode(pinPos, INPUT);
    if (isDifferential) pinMode(pinNeg, INPUT);
}

float VoltageSensor::readVoltage() {
    if (isDifferential) {
        int rawPos = analogRead(pinPos);
        int rawNeg = analogRead(pinNeg);
        
        // Calculate step delta across the differential pair
        float stepDelta = (float)(rawPos - rawNeg);
        float pinDeltaV = stepDelta * (5.0f / 1024.0f);
        
        return pinDeltaV * scaleFactor; // Multiplies by your 250x factor
    } else {
        int raw = analogRead(pinPos);
        float pinV = raw * (5.0f / 1024.0f);
        
        return pinV * scaleFactor;      // Multiplies by your 22.36x divider ratio
    }
}