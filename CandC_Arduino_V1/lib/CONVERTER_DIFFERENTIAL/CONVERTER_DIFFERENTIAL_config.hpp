#include <Arduino.h>

constexpr float v_ref = 5.0;

//Pins differential voltage
constexpr uint8_t differentialVoltage_VA_PIN = A3;
constexpr uint8_t differentialVoltage_VB_PIN = A5;
//According to data sheet
constexpr float differentialVotlage_scaleFactor = 125.0;

//IIR filter constant
constexpr float filterParamerter = 0.1;