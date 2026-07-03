#pragma  once
#include "TEMP_SENSOR_config.hpp"


void init_tempSensor(tempSensors_ID_t ID);

void tempSensor_request(tempSensors_ID_t ID);

float tempSensor_measurment(tempSensors_ID_t ID);