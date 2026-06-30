#include "TEMP_SENSOR_config.hpp"


const tempSensor_config_t tempSensor_config[TEMP_SENSOR_COUNT]{
    {temperature_1_pin, 9, 0.10f},
    {temperature_2_pin, 9, 0.10f}
};

tempSensor_state_t tempSensor_state[TEMP_SENSOR_COUNT] = {
    {0.0f,0,false},
    {0.0f,0,false}
};