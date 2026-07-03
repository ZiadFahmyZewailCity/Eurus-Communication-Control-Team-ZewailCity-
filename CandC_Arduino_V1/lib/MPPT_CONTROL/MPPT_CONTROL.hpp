#include <Arduino.h>

// Run once before using MPPT
// Configures Pin 6 and Timer 4 for 35 kHz Fast PWM
void configureMPPT();

// Pass this function the voltage and current before the converter
// It will output a duty cycle from 0.0 to 1.0
float MPPT(float V, float I);

// Converts float duty cycle to the 0 - 456 Timer range
// If value greater than 1.0, returns MPPT_TIMER_TOP
// If value less than 0.0, returns 0
uint16_t computeTimerOCR(float dutyCycle);

// Directly writes the timer value to the OCR4A hardware register
void applyPWM(uint16_t ocrValue);