#include "RPM_ENCODER.hpp"
#include "RPM_ENCODER_config.hpp"


static volatile int32_t encoder_pulse_count = 0;
static unsigned long lastTime_ms = 0;


static void encoderISR_phaseA(void){

    //Checks to see if its high or low dictating if a positive or negative pulse
    //has been recieved
    if(digitalRead(RPM_ENCODER_PIN_B) == HIGH) {

        encoder_pulse_count++;
    }
    else{

        encoder_pulse_count--;
    }

}

void RPM_ENCODER_config(void)
{

   pinMode(RPM_ENCODER_PIN_A,INPUT);
   pinMode(RPM_ENCODER_PIN_B,INPUT);

    encoder_pulse_count = 0;
    lastTime_ms = millis();

    attachInterrupt(digitalPinToInterrupt(RPM_ENCODER_PIN_A),encoderISR_phaseA,RISING);

}

float RPM_ENCODER_getValue(void)
{

    unsigned long currentTime_ms = millis();
    unsigned long deltaTime_ms = currentTime_ms - lastTime_ms;

    //Stop divide by zero error
    if(deltaTime_ms == 0)
    {
        return 0.0f;
    }


    //To read the encoder pulses current properly we need 
    //to temporarily disable interrupts
    noInterrupts();
    int32_t current_pulses = encoder_pulse_count;
    encoder_pulse_count = 0;
    interrupts();

    //Set new reference point to last time
    lastTime_ms = currentTime_ms;

    return ((float)current_pulses/ RPM_ENCODER_PPR) * (60000.0f / deltaTime_ms);

}