#include <Arduino.h>


//Voltage Divider PIN configuration
constexpr uint8_t voltageDividerPin = A3;


//Resistors
constexpr float R1_high_side = 47000.0f; 
constexpr float R2_low_side  = 2200.0f;

//Constant factor
constexpr float voltageDividerFactor = R2_low_side / (R1_high_side + R2_low_side);