#pragma once
#include "ACS12_SENSOR_config.hpp"


//This stores the configuration parameters of the current sensor
//These will remain constant and will remain in ROM
const currentSensor_config_t currentSensor_config[SENSOR_COUNT] = {

    //Configruation of current sensors
    {pre_converter_pin,ACS712_20_SENSITIVITY,0.15f}, //Sensor Pre converter config
    {post_converter_pin,ACS712_20_SENSITIVITY,0.15f} //Sensor Post converter config
};


//This stores the current state of the current sensor
//These variables are varying and will be stored in RAM
currentSensor_state_t currentSensor_state[SENSOR_COUNT] { 
    {0,0.0f}, //Sensor parameters Pre converter
    {0,0.0f} //Sensor parameters Post converter
};