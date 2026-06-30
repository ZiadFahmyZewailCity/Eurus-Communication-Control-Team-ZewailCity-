#pragma
#include "ACS12_SENSOR_config.hpp"


//This function configures the zero point of the current sensor
//Ensure that either you are sure zero current is flowing (Such as an open circuit)
void currentSensor_intialize(currentSensors_ID_t ID);
bool currentSensor_calibrate_blocking(currentSensors_ID_t ID);
float currentSensor_measurment(currentSensors_ID_t ID);


