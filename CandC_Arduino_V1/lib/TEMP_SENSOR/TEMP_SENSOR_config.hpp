#pragma once
#include <Arduino.h>
#include "../../include/pinDefinitions.hpp"\


typedef enum {

    TEMP_SENSOR_1 = 0,
    TEMP_SENSOR_2 = 1,
    TEMP_SENSOR_COUNT

} tempSensors_ID_t;

typedef struct {

    uint8_t pin;
    //Resolution which can be set by DS18B20 (9 -> 12 bit)
    uint8_t resolution;
    //IIR filter weight
    float filterWeight;

} tempSensor_config_t;


typedef struct {

    float temp_filtered;
    //Track when is the last time the program asked for a temperature reading
    unsigned long last_request_time;
    //State flag
    bool is_converting;

} tempSensor_state_t;

//Tell linker that these arrays memory wont be allocated here
extern const tempSensor_config_t tempSensor_config[TEMP_SENSOR_COUNT];
extern tempSensor_state_t tempSensor_state[TEMP_SENSOR_COUNT];