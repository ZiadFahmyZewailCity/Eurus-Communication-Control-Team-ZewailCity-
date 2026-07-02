#include <Arduino.h>
#include "pinDefinitions.hpp"

constexpr float v_ref = 5.0;

//According to data sheet
constexpr float differentialVotlage_scaleFactor = 125.0;

//IIR filter constant
constexpr float filterParamerter = 0.1;