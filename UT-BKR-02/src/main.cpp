#include <Arduino.h>

// int myFunction(int, int);

//Pin Definitions
#define pin_DIR 7
#define pin_PWM 6

void setup() {
  

  Serial.begin(9600);

  //Setting both pins to output 
  pinMode(pin_DIR,OUTPUT);
  pinMode(pin_PWM,OUTPUT);

  //SET POLARITY FROM THIS VARIABLE
  digitalWrite(pin_DIR,LOW);
  analogWrite(pin_PWM,0);

}

void loop() 
{

  int targetPercent = 0;

    if(Serial.available() > 0){

      // Clear rest of the serial buffer 
      targetPercent = Serial.parseInt();
      while(Serial.available() > 0){

        Serial.read();
      }

      
    if (targetPercent >= 0 && targetPercent <= 100)
    {
      //takes the percentage and turns it to a PWM signal
      uint8_t pwmValue = map(targetPercent,0,100,0,255);

      Serial.print("% Duty Cycle: ");
      Serial.print(targetPercent);

      //Output signal
      analogWrite(pin_PWM,pwmValue);

    }
    else
    {
      Serial.print("Invalid input");
    }
    
  }
}



