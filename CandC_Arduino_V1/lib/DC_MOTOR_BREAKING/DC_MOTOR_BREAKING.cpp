#include <Arduino.h>
#include "DC_MOTOR_BREAKING_config.hpp"
#include "DC_MOTOR_BREAKING.hpp"

//Target angular position
static float target_pos_rads = TARGET_POS_RADS;
static float current_pos_rads = 0.0f;
static float prev_error = 0.0f;

static volatile long encoderCount = 0;
static unsigned long last_update_time = 0;

void configureBrakeSystem(void)
{

    //Set pins
    //Encoder Pins
    pinMode(pin_ENCA,INPUT);
    pinMode(pin_ENCB,INPUT);
    attachInterrupt(digitalPinToInterrupt(pin_ENCA), updateEncoder,RISING);

    //Driver Pins
    pinMode(pin_DIR,OUTPUT);
    pinMode(pin_PWM,OUTPUT);


    //Set intital states
    digitalWrite(pin_DIR,LOW);
    analogWrite(pin_PWM,0);

    last_update_time = millis();

}



float getPosition()
{
    return current_pos_rads;
}

float getError()
{
    return target_pos_rads - current_pos_rads;
}

void updateBrakingLoop(){

    //Get current time
    unsigned long current_time = millis();

    //Enforce sampling time to be like simulink simulation
    if(current_time - last_update_time  < (unsigned long)( samplingTime*1000 ) )
    {
        return;
    }

    last_update_time = current_time;

    noInterrupts();
    long brakePosition = encoderCount;
    interrupts();

    //Get current position given the previous snapshot of the position
    current_pos_rads = ((float)brakePosition / TOTAL_CPR) * 2.0f * PI;
    //Error computing
    float error = target_pos_rads - current_pos_rads;
    //Derivative of the error
    float derivative = (error - prev_error) / samplingTime;

    //Control System output
    float controlSystemOutput = (kp * error) + (kd * derivative);
    //Constrain Voltage output 
    float constrainedOutput = constrain(controlSystemOutput, -v_rated, v_rated);

    //Identify direction
    if (constrainedOutput >= 0){

        digitalWrite(pin_DIR,FORWARD);
    }
    else
    {
        digitalWrite(pin_DIR,BACKWARD);
    }

    int pwmValue = map(abs(constrainedOutput) * 1000,0,v_rated*1000,0,255);

    analogWrite(pin_PWM,pwmValue);
    prev_error = error;
}

void updateEncoder(){

    if(digitalRead(pin_ENCB) == HIGH){
        encoderCount--;
    }
    else{
        encoderCount++;
    }


}
