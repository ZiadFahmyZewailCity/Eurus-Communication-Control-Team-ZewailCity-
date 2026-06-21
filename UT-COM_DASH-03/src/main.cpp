#include <Arduino.h>
#include "BluetoothSerial.h"


//Dummy variables being sent
//Group A
float rpm = 310.0;
//Group B
float pitchAngle = 12.5;
//Group C
float temperature = 42.0;
//Command signals from dashboard to turbine
bool stopRequested = false;
bool stopAck = false;


//A work around for this should be found, will eventually run out of space
unsigned long lastMasterTick = 0;
//Sending intervals
const unsigned long masterInterval = 500;
//Tick Counter to keep track of when to append 
unsigned int tickCounter = 0;

BluetoothSerial SerialBT;

//Function to generate random numbers for parameters
void triggerSensorA() { rpm = 310.0 + random(-10, 11) * 0.1; }
void triggerSensorB() { pitchAngle = 12.5 + random(-5, 6) * 0.1; }
void triggerSensorC() { temperature = 42.0 + random(-2, 3) * 0.1; }
//Function to generate payload
String generate_payLoad(const char& statusA, const char& statusB, const char& statusC, const float rpm, const float pitch, const float temp, const char stopACK)
{
  //TO DO: Double check i understand the stopACK syntax, i think its just a return based on what type of bool is in the stop ack vairable, but if thats the case i should be just able to place the variable 
  //directly and when it gets turned into a string itll be either 0 || 1
  return String("$EURUS") + "," + statusA + "," + statusB + "," + statusC + "," + String(rpm,1) + "," + String(pitch,1) + "," + String(temp,1) + "," + String(stopACK ? 1 : 0);
}

void setup() {

  //Configure Serial connection
  Serial.begin(9600);
  SerialBT.begin("EURUS_COMHUB_ESP32");
  Serial.println("Bluetooth Started");

}

void loop() {

  //Reading command signals
  if(SerialBT.available()){

    //TO DO: Dont fully understand what these two lines do
    String incoming = SerialBT.readStringUntil('\n');
    //Maybe removed header
    incoming.trim();


    //Stop Signal
    if(incoming == "S"){
      stopRequested = true;
      stopAck = true;
      Serial.println("MANUAL STOP TRIGGERED");

    }
    //Reset Stop Signal
    else if (incoming == "R"){
      stopRequested = false;
      stopAck = false;
      Serial.println("RESET SENT, Breaks disengaged");

    }




  }


  if (millis() - lastMasterTick >= masterInterval) {
    lastMasterTick = millis();
    tickCounter++;

    char statusA = 'A'; 
    triggerSensorA();

    char statusB = 'F'; 
    if (tickCounter % 2 == 0) {
      statusB = 'B'; 
      triggerSensorB();
    }

    char statusC = 'F'; 
    if (tickCounter % 120 == 0) {
      statusC = 'C'; 
      tickCounter = 0; 
      triggerSensorC();
    }

    String payLoad = generate_payLoad(statusA,statusB,statusC,rpm,pitchAngle,temperature,stopAck);


    SerialBT.println(payLoad);

  }

}