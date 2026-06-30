#include "ACS12_SENSOR.hpp"


void currentSensor_intialize(currentSensors_ID_t ID)
{
    pinMode(currentSensor_config[ID].pin,INPUT);
}

bool currentSensor_calibrate_blocking(currentSensors_ID_t ID)
{

    //Read from ROM 
    uint8_t currentSensorPin = currentSensor_config[ID].pin;
    
    //Take an average of samples when floating
    double samples = 0;
    for(int i = 0; i < 500; i++)
    {
        float samples =+ analogRead(currentSensorPin);
        delay(2);
    }

    //Attach the zero value to the currentSensor struct
    currentSensor_state[ID].zeroPoint_ADCVAL = samples/500;
    return true;

}

float currentSensor_measurment(currentSensors_ID_t ID)
{

    //ROM Variables
    uint8_t pin = currentSensor_config[ID].pin;
    float senstivity = currentSensor_config[ID].sensitivity;
    float filterParameter = currentSensor_config[ID].alpha;

    //RAM Variables
    float zeroPoint_ADC = currentSensor_state[ID].zeroPoint_ADCVAL;
    float filteredValue = currentSensor_state[ID].filtered_current;

    //Measure Current value
    uint16_t rawVoltage = analogRead(pin);

    //Convert to voltage (This value is highly dependent on v_ref, check it)
    float voltage = (rawVoltage / 1024) * v_ref;

    //Convert to current
    float raw_current = (voltage - zeroPoint_ADC)/senstivity;

    //Current after applying filter
    float newFiltered = (filterParameter*raw_current) + ((1.0f - filterParameter) * filteredValue);

    currentSensor_state[ID].filtered_current = newFiltered;

    return newFiltered;

}

