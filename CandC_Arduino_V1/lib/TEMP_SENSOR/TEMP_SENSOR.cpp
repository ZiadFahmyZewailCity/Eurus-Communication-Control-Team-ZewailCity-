#include "TEMP_SENSOR.hpp"
#include <OneWire.h>
#include <DallasTemperature.h>

// By making these static, they have internal linkage. 
// They are completely invisible to any other file in your project.
static OneWire* oneWire_buses[TEMP_SENSOR_COUNT] = {nullptr};
static DallasTemperature* sensor_instances[TEMP_SENSOR_COUNT] = {nullptr};

// Helper: Maps resolution to required conversion time
static uint16_t getConversionTimeMs(uint8_t resolution) {
    switch(resolution) {
        case 9:  return 94;
        case 10: return 188;
        case 11: return 375;
        case 12: 
        default: return 750;
    }
}

void init_tempSensor(tempSensors_ID_t ID) {
    if (ID >= TEMP_SENSOR_COUNT) return;

    // Allocate library objects once using the pin from the config array
    if (oneWire_buses[ID] == nullptr) {
        // FIXED: Changed init_tempSensor to tempSensor_config
        oneWire_buses[ID] = new OneWire(tempSensor_config[ID].pin);
        sensor_instances[ID] = new DallasTemperature(oneWire_buses[ID]);
    }

    sensor_instances[ID]->begin();
    
    // Set the configured resolution (9 to 12 bits)
    // FIXED: Changed init_tempSensor to tempSensor_config
    sensor_instances[ID]->setResolution(tempSensor_config[ID].resolution);
    
    // CRITICAL: Disable blocking delay in the library
    sensor_instances[ID]->setWaitForConversion(false);

    // Initialize state
    tempSensor_state[ID].is_converting = false;
    tempSensor_state[ID].last_request_time = 0;
    tempSensor_state[ID].temp_filtered = 0.0f; 
}

void tempSensor_request(tempSensors_ID_t ID) {
    if (ID >= TEMP_SENSOR_COUNT) return;

    // Only request if we aren't already waiting for a conversion
    if (!tempSensor_state[ID].is_converting) {
        sensor_instances[ID]->requestTemperatures();
        tempSensor_state[ID].last_request_time = millis();
        tempSensor_state[ID].is_converting = true;
    }
}

float tempSensor_measurment(tempSensors_ID_t ID) {
    if (ID >= TEMP_SENSOR_COUNT) return 0.0f;

    if (tempSensor_state[ID].is_converting) {
        
        // FIXED: Changed init_tempSensor to tempSensor_config
        uint16_t delay_required = getConversionTimeMs(tempSensor_config[ID].resolution);

        // Check if the required conversion time has elapsed
        if ((millis() - tempSensor_state[ID].last_request_time) >= delay_required) {
            
            // Read the raw temperature. 
            float raw_temp = sensor_instances[ID]->getTempCByIndex(0); 

            // Handle sensor error (-127.0C is the standard DS18B20 error code)
            if (raw_temp != DEVICE_DISCONNECTED_C) {
                
                // Apply IIR Filter
                if (tempSensor_state[ID].temp_filtered == 0.0f) {
                    tempSensor_state[ID].temp_filtered = raw_temp;
                } else {
                    // FIXED: Changed init_tempSensor to tempSensor_config
                    float w = tempSensor_config[ID].filterWeight;
                    tempSensor_state[ID].temp_filtered = (raw_temp * w) + (tempSensor_state[ID].temp_filtered * (1.0f - w));
                }
            }

            // Reset state so a new request can be made
            tempSensor_state[ID].is_converting = false;
        }
    }
    
    return tempSensor_state[ID].temp_filtered;
}