#pragma once

#include <Arduino.h>
#include "../../include/pinDefinitions.hpp"

constexpr float ACS712_20_SENSITIVITY = 0.100f;
constexpr float ACS712_30_SENSITIVITY = 0.066f;
constexpr float ACS712_5_SENSITIVITY  = 0.185f;

constexpr float v_ref = 5.0;


typedef enum {
    SENSOR_1 = 0,
    SENSOR_2 = 1,
    SENSOR_COUNT
} currentSensors_ID_t;


//These are essentially read only, to be set in ROM
typedef struct {
    uint8_t pin;
    float sensitivity;
    float alpha; // Filter Weight
} currentSensor_config_t;


typedef struct {

    //This is the value the ADCs output at zero volt given proper configuration
    int zeroPoint_ADCVAL;
    //Value of current after filter
    float filtered_current;

} currentSensor_state_t;

//States that the mememory allocation for this array will exist in a differet file

//Array for current sensor configs
extern const currentSensor_config_t currentSensor_config [SENSOR_COUNT];
//Array for current sensoro states
extern currentSensor_state_t currentSensor_state [SENSOR_COUNT];
