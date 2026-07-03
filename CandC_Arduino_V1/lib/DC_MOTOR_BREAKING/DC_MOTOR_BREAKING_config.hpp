#pragma once
#include <Arduino.h>
#include "../../include/pinDefinitions.hpp"
//Encoder CPR
constexpr float TOTAL_CPR = 2206.0f;
//Motor Voltage
constexpr float v_rated = 12.0f;
//10ms Sampling Time
constexpr float samplingTime = 0.01f;

//PD Controller Gains
constexpr float kp = 4.101f;
constexpr float kd = 0.0205f;

//Target Displacement
constexpr float TARGET_POS_RADS = 5.0f;

//Configure as needed 
//Whatever is considered to be forward and backward change 
//High and low accordingly
constexpr uint8_t FORWARD = LOW;
constexpr uint8_t BACKWARD = HIGH;





