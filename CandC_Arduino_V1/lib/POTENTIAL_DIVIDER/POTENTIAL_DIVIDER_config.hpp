#pragma once
#include <Arduino.h>
#include "../../include/pinDefinitions.hpp"
//Voltage value is highly dependent on reference voltage, make sure this is well tuned 
constexpr float v_ref = 5.0;

//Resistors
constexpr float R1_high_side = 47000.0f; 
constexpr float R2_low_side  = 2200.0f;

//Constant factor
constexpr float voltageDividerFactor = (R1_high_side + R2_low_side) / R2_low_side;