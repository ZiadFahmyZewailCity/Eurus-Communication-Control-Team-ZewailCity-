// mppt.cpp
#include <Arduino.h>
#include "mppt.hpp"
#include "mppt_config.hpp"


float V_old = 0;
float I_old = 0;
float D_old = 0.5;

float MPPT(float V, float I) {
  float dV = V - V_old;
  float dI = I - I_old;
  float D;

  if (abs(dV) < 0.0001) {
    if (dI > 0) D = D_old - MPPT_STEP;
    else if (dI < 0) D = D_old + MPPT_STEP;
    else D = D_old;
  } else {
    float slope = dI / dV;
    float cond = -I / V;

    if (abs(slope - cond) < 0.001)
      D = D_old;
    else if (slope > cond)
      D = D_old - MPPT_STEP;
    else
      D = D_old + MPPT_STEP;
  }

  // Limits (Using the constants from config.hpp)
  if (D > MPPT_MAX_DUTY) D = MPPT_MAX_DUTY;
  if (D < MPPT_MIN_DUTY) D = MPPT_MIN_DUTY;

  // Update memory
  V_old = V;
  I_old = I;
  D_old = D;

  return D;
}