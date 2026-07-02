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
  float D = D_old;
  bool stepped = false;

  // 1. ADC Quantization Deadbands
  const float V_DEADBAND = 0.05f; 
  const float I_DEADBAND = 0.02f;
  
  // 2. Conductance Tolerance
  const float IC_TOLERANCE = 0.05f;

  if (abs(dV) < V_DEADBAND) {
    if (dI > I_DEADBAND) { 
        D = D_old - MPPT_STEP; 
        stepped = true; 
    }
    else if (dI < -I_DEADBAND) { 
        D = D_old + MPPT_STEP; 
        stepped = true; 
    }
  } else {
    // Standard Incremental Conductance Logic
    float slope = dI / dV;
    float cond = -I / V;

    if (abs(slope - cond) < IC_TOLERANCE) {
      // At MPP, do nothing
    } 
    else if (slope > cond) {
      D = D_old - MPPT_STEP;
      stepped = true;
    } 
    else {
      D = D_old + MPPT_STEP;
      stepped = true;
    }
  }

  // Limits
  if (D > MPPT_MAX_DUTY) D = MPPT_MAX_DUTY;
  if (D < MPPT_MIN_DUTY) D = MPPT_MIN_DUTY;

  // 3. Anchor Memory (The Fix)
  // Only update history if the algorithm stepped, or if 
  // the environment changed enough to break the deadband thresholds.
  if (stepped || abs(dV) >= V_DEADBAND || abs(dI) >= I_DEADBAND) {
    V_old = V;
    I_old = I;
  }
  
  // Always update the duty cycle memory
  D_old = D;

  return D;
}