#include <Arduino.h>
#pragma once

//Run once to configure the breaking systems parameters 
//Setup breaking system ISR for encoder
//Setup PINs
//Starting State
void configureBrakeSystem(void);

//Call this every loop
void updateBrakingLoop(void);

//Brake
void stopBrakeMotorDriver(void);

//Return position & Error
float getPosition();
float getError();

//ISR needed to track encoder count, called on interrupts 
void updateEncoder();

