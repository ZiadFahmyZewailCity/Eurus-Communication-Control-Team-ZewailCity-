#include <Arduino.h>
#include "../../include/pinDefinitions.hpp"

//MPPT Parameters
constexpr float MPPT_MIN_DUTY = 0.15f;
constexpr float MPPT_MAX_DUTY = 0.95f;
constexpr float MPPT_STEP = 0.005;

// Timer Hardware Parameters (35 kHz Target)
// TOP = (16MHz / (1 * 35kHz)) - 1 = 456
constexpr uint16_t MPPT_TIMER_TOP = 456;