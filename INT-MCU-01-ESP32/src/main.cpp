#include <Arduino.h>
#include "BluetoothSerial.h"
#include "internal_telemetry_packet.hpp"


//UART Communication with Arduino 
#define RXD2 16
#define TXD2 17


// --- GROUP A PARAMETERS (2 Hz Master) ---
float rpm;     
float power;
float currentConv;  
float voltageConv;  
float currentRect;  
float voltageRect;

// --- GROUP B PARAMETER (1 Hz) ---
float pitchAngle;

// --- GROUP C PARAMETER (1/60 Hz) ---
float temperature;

// Command signals 
bool stopRequested = false;
u_int8_t stopAck = 0;

internal_payLoad arduinoPayload_buffer;

//Traking sending of data at given intervals
//Technically this could eventually run out, keep in mind
unsigned long lastMasterTick = 0;
const unsigned long masterInterval = 500;
unsigned int tickCounter = 0;

//Object for managing serial communication
BluetoothSerial SerialBT;


// PayLoad generator
String generate_payLoad(char statusA, char statusB, char statusC, 
                        float rpm, float pwr, float cConv, float vConv, float cRect, float vRect, 
                        float pitch, float temp,  u_int8_t isAck) 
{
  return String("$EURUS") + "," + 
         statusA + "," + statusB + "," + statusC + "," + 
         String(rpm, 1)   + "," + 
         String(pwr, 1)   + "," + 
         String(cConv, 2) + "," + 
         String(vConv, 2) + "," + 
         String(cRect, 2) + "," + 
         String(vRect, 2) + "," + 
         String(pitch, 1) + "," + 
         String(temp, 1)  + "," + 
         //Double check i understand this syntax
         String(isAck ? 1 : 0);
}

void setup() {
  Serial.begin(9600); 
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  
  SerialBT.begin("EURUS_COMHUB_ESP32");
  Serial.println("Eurus Telemetry Bridge Online.");
}

void loop() {

  //Reading raw dataPacket from Arduino
  if(Serial2.available() >= sizeof(internal_payLoad)){

    //Check for pre-defined synchronization byte, agreed to be 0xAA before both devices
    //Peak takes a look without emptying buffer
    if(Serial2.peek() == 0xAA){

      //Packet confirmed
      //Double check that the first input, is a ptr to the address of the payload
      //Check why this can be a ptr to a uint8_t and not the type of internal_payLoad
      Serial2.readBytes((uint8_t*)&arduinoPayload_buffer, sizeof(internal_payLoad));

      if(arduinoPayload_buffer.syncByte_2 == 0xBB){

        stopAck     = arduinoPayload_buffer.brakeACK;
        rpm         = arduinoPayload_buffer.rpm;
        power       = arduinoPayload_buffer.power;
        currentConv = arduinoPayload_buffer.currentConv;
        voltageConv = arduinoPayload_buffer.voltageConv;
        currentRect = arduinoPayload_buffer.currentRect;
        voltageRect = arduinoPayload_buffer.voltageRect;
        pitchAngle  = arduinoPayload_buffer.pitchAngle;
        temperature = arduinoPayload_buffer.temperature;
        Serial.println("Valid packet recieved and read from buffer");
      }
      else{

      }

    }
    else
    {
      //Skip this malformed packet move to next
      Serial2.read();
    }




  }

  //Handling Stop signals from dashboard
  if (SerialBT.available()) {
    String incoming = SerialBT.readStringUntil('\n');
    incoming.trim();

    if (incoming == "S") {
      stopRequested = true;
      Serial2.write('S');

    }
    /*
    else if (incoming == "R") {
      stopRequested = false;
      stopAck = false;
      Serial.println("SAFETY: RESET SENT, Brakes disengaged");
    }
    */
  }

  //Send data to dashboard
  if (millis() - lastMasterTick >= masterInterval) {
    lastMasterTick = millis();
    tickCounter++;

    // Group A (2 Hz)
    char statusA = 'A'; 
    

    // Group B (1 Hz)
    char statusB = 'F'; 
    if (tickCounter % 2 == 0) {
      statusB = 'B'; 
     
    }

    // Group C (1/60 Hz)
    char statusC = 'F'; 
    if (tickCounter % 120 == 0) {
      statusC = 'C'; 
      tickCounter = 0; 
    
    }

    String payLoad = generate_payLoad(statusA, statusB, statusC, 
                                      rpm, power, currentConv, voltageConv, currentRect, voltageRect, 
                                      pitchAngle, temperature, stopAck);

    SerialBT.println(payLoad);
  }
}