#include <Arduino.h>
#include "MPPT_CONTROL_config.hpp"

//MPPT Values 
float V_old = 0;
float I_old = 0;
float D_old = 0.5;

void configureMPPT(){
    pinMode(MPPT_PIN,OUTPUT);
}

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


  if (D > MPPT_MAX_DUTY) D = MPPT_MAX_DUTY;
  if (D < MPPT_MIN_DUTY) D = MPPT_MIN_DUTY;

  // Update memory
  V_old = V;
  I_old = I;
  D_old = D;

  return D;
}

uint8_t computePWM(float dutyCycle)
{
    if(dutyCycle > 1) { return 255; }
    else if(dutyCycle < 0) { return 0; }
    else
    {

       //add the half to prevent trunking 
       return (uint8_t)(dutyCycle * 255.0f + 0.5f);

    }
}