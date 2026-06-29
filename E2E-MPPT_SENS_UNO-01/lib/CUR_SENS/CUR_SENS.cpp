#include "CUR_SENS.hpp"

// The constructor initializes the private variables
CurrentSensor::CurrentSensor(uint8_t analogPin, float referenceVoltage, float sensorSensitivity) {
    pin = analogPin;
    vref = referenceVoltage;
    sensitivity = sensorSensitivity;
    
    zeroPoint_ADCVAL = 512;
    filtered_current = 0.0f;
    alpha = 0.15f;          
}

void CurrentSensor::begin() {

    pinMode(pin, INPUT);
}

//Calibrating sensor function
bool CurrentSensor::calibrateCurrentSensor() {
    long sum = 0;
    
    for(int i=0; i<500; i++){
        sum += analogRead(pin); 
        delay(2); 
    }
    
    zeroPoint_ADCVAL = sum / 500; 
    return true;
}

float CurrentSensor::readCurrent() {
    int rawValue = analogRead(pin);
    
    //Calibrated ADC
    float adc_difference = rawValue - zeroPoint_ADCVAL;
    
    //Voltage difference 
    float voltage_difference = adc_difference * (vref / 1024.0f);
    
    //Voltage different to current
    float raw_current = voltage_difference / sensitivity;

    // 4. Apply the EMA filter to smooth out Hall-effect noise
    filtered_current = (alpha * raw_current) + ((1.0f - alpha) * filtered_current);
    return filtered_current;
}