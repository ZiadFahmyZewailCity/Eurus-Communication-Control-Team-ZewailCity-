#include <Arduino.h>

//Run once before using MPPT
//Set MPPT PIN to output
void configureMPPT();


//Pass this function the voltage and current before the connverter
//it will output a duty cycle which you should convert to a value from 0 - 255
float MPPT(float V, float I);


//Converts duty cycle to a value for Arduino Analog write (PWM)
//If value greater than 1 return 255
//if value less than 0 return 0
uint8_t computePWM(float dutyCycle);