#include "MPPT_CONTROL.hpp"
#include "MPPT_CONTROL_config.hpp"

// MPPT State Memory
float V_old = 0;
float P_old = 0; // We track absolute Power now instead of Current
float D_old = 0.5;

void configureMPPT(){
    pinMode(MPPT_PIN, OUTPUT); 

    // Clear Timer 4 control registers
    TCCR4A = 0;
    TCCR4B = 0;

    // 1. Set Fast PWM mode (Mode 14), where TOP is defined by ICR4
    TCCR4A |= (1 << WGM41);
    TCCR4B |= (1 << WGM42) | (1 << WGM43);

    // 2. Enable Non-Inverting PWM on Channel A (Pin 6)
    TCCR4A |= (1 << COM4A1);

    // 3. Set Prescaler to 1 (No prescaling, timer runs at 16 MHz)
    TCCR4B |= (1 << CS40);

    // 4. Set TOP value for 35 kHz target (456)
    ICR4 = MPPT_TIMER_TOP;

    // 5. Initialize duty cycle to 0 output
    OCR4A = 0;
}

float MPPT(float V, float I) {
  float P = V * I;
  float dV = V - V_old;
  float dP = P - P_old;
  float D = D_old;

  // Realistic noise deadbands based on 10-bit ADC and ACS712 resolution
  // Require at least 0.5W change and 0.2V change to confidently act
  if (abs(dP) > 0.50f && abs(dV) > 0.20f) { 
    
    if (dP > 0) {
      // Power went UP. Keep going in the same voltage direction.
      if (dV > 0) {
        D = D_old - MPPT_STEP; // To increase V_in, decrease load (Duty Cycle)
      } else {
        D = D_old + MPPT_STEP; // To decrease V_in, increase load (Duty Cycle)
      }
    } else {
      // Power went DOWN. Reverse the voltage direction.
      if (dV > 0) {
        D = D_old + MPPT_STEP; // V_in went up, P went down. Need to decrease V_in.
      } else {
        D = D_old - MPPT_STEP; // V_in went down, P went down. Need to increase V_in.
      }
    }
  }

  // Enforce hardware duty cycle limits
  if (D > MPPT_MAX_DUTY) D = MPPT_MAX_DUTY;
  if (D < MPPT_MIN_DUTY) D = MPPT_MIN_DUTY;

  // Update memory for next frame
  V_old = V;
  P_old = P;
  D_old = D;

  return D;
}

uint16_t computeTimerOCR(float dutyCycle)
{
    if(dutyCycle >= 1.0f) { return MPPT_TIMER_TOP; }
    else if(dutyCycle <= 0.0f) { return 0; }
    else
    {
       // Scale 0.0-1.0 to 0-456, add 0.5 to round properly
       return (uint16_t)(dutyCycle * (float)MPPT_TIMER_TOP + 0.5f);
    }
}

void applyPWM(uint16_t ocrValue) {
    OCR4A = ocrValue; 
}