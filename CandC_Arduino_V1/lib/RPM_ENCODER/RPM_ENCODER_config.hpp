#pragma once
#include <Arduino.h>
#include "../../include/pinDefinitions.hpp"


//PPR according to datasheet
constexpr float RPM_ENCODER_PPR = 600;
//Ratio for pulley attached to encoder
constexpr float RPM_PULLEY_RATIO = 1.5;