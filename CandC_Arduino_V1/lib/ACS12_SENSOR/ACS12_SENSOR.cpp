#include "ACS12_SENSOR.hpp"

void currentSensor_intialize(currentSensors_ID_t ID)
{
    pinMode(currentSensor_config[ID].pin, INPUT);
}

bool currentSensor_calibrate_blocking(currentSensors_ID_t ID)
{
    //Read from ROM 
    uint8_t currentSensorPin = currentSensor_config[ID].pin;
    
    float samples = 0; 
    for (int i = 0; i < 500; i++) {
        samples += analogRead(currentSensorPin); 
    }

    //Attach the zero value to the currentSensor struct
    // Cast to int since the struct expects an integer ADC value
    currentSensor_state[ID].zeroPoint_ADCVAL = (int)(samples / 500.0f);
    return true;
}

float currentSensor_measurment(currentSensors_ID_t ID)
{
    //ROM Variables
    uint8_t pin = currentSensor_config[ID].pin;
    float sensitivity = currentSensor_config[ID].sensitivity;
    float filterParameter = currentSensor_config[ID].alpha;

    //RAM Variables
    float zeroPoint_ADC = (float)currentSensor_state[ID].zeroPoint_ADCVAL;
    float filteredValue = currentSensor_state[ID].filtered_current;

    //Measure Current value
    uint16_t rawVoltage = analogRead(pin);

    // Convert raw ADC reading to voltage (using float division)
    float voltage = (rawVoltage / 1024.0f) * v_ref;

    // Convert the calibrated zero point ADC value to voltage
    float zeroPoint_Voltage = (zeroPoint_ADC / 1024.0f) * v_ref;

    // Convert to current (both operands are now in Volts)
    float raw_current = (voltage - zeroPoint_Voltage) / sensitivity;

    //Current after applying IIR filter 
    float newFiltered = (filterParameter * raw_current) + ((1.0f - filterParameter) * filteredValue);

    currentSensor_state[ID].filtered_current = newFiltered;

    return newFiltered;
}