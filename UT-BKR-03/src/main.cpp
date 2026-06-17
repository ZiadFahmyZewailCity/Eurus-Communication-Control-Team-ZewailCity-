#include <Arduino.h>


#define ENCA 2
#define ENCB 3


//MOTOR GEAR RATIO
//Gear Ratio 1:87
//PPR 11


#define TOTAL_CPR 957

// volatile is required for variables modified inside an interrupt
volatile long encoderCount = 0; 


void setup() {
  Serial.begin(9600);
  
  //Setting interrupt pins
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);

  //Interrupt is triggered on 
  attachInterrupt(digitalPinToInterrupt(ENCA), updateEncoder, RISING);
}

void loop() {
  
  Serial.print("Position: ");
  Serial.println(encoderCount);
  delay(100); 
}

// ISR
void updateEncoder() {

  // Read the state of Phase B to determine direction
  if (digitalRead(ENCB) == HIGH) {

    //Forward
    encoderCount++; 
  } else {

    //Backward
    encoderCount--; 
  }
}