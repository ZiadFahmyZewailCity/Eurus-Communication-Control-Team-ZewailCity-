#include <Arduino.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include "BluetoothSerial.h"
#include "internal_telemetry_packet.hpp"

// --- HARDWARE UART DEFINITIONS ---
#define RXD2 16
#define TXD2 17

// --- WI-FI & THINGSPEAK CREDENTIALS (FILL THESE IN) ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
unsigned long myChannelNumber = 12345678;         // Replace with your ThingSpeak Channel ID
const char* myWriteAPIKey = "YOUR_WRITE_API_KEY"; // Replace with your Write API Key

WiFiClient client;

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

//Tracking sending of data at given intervals
unsigned long lastMasterTick = 0;
const unsigned long masterInterval = 500;
unsigned int tickCounter = 0;

// Tracking Group D (Table 27 Long-Term Storage @ 1 min^-1)
unsigned long lastGroupDTick = 0;
const unsigned long groupDInterval = 60000; 

//Object for managing serial communication
BluetoothSerial SerialBT;
//Arduino payload 
internal_payLoad arduinoPayload_buffer;


//This function takes the data give from the UART communication from the Arduino and packages into a packet for wireless communication
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
         String(isAck ? 1 : 0);
}


void setup() {

  Serial.begin(9600); 
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  
  SerialBT.begin("Eurus Gateway");
  Serial.println("Eurus Telemetry Bridge Online.");

  // Start Wi-Fi & Connect to ThingSpeak Client
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  ThingSpeak.begin(client);

}

void loop() {

  // ------------------------------------------------------------------
  // 1. INCOMING UART PIPELINE (From Arduino)
  // ------------------------------------------------------------------
  if(Serial2.available() >= sizeof(internal_payLoad)){

    if(Serial2.peek() == 0xAA){

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
        // Corrupted payload tail
      }
    }
    else
    {
      // Skip this malformed byte to re-align the buffer header
      Serial2.read();
    }
  }

  // ------------------------------------------------------------------
  // 2. HANDLING STOP SIGNALS FROM DASHBOARD (BLUETOOTH)
  // ------------------------------------------------------------------
  if (SerialBT.available()) {
    String incoming = SerialBT.readStringUntil('\n');
    incoming.trim();

    if (incoming == "S") {
      stopRequested = true;
      Serial2.write('S');
    }
  }

  // ------------------------------------------------------------------
  // 3. REAL-TIME DISPATCH: MATLAB BLUETOOTH DASHBOARD (500ms / 2Hz)
  // ------------------------------------------------------------------
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

  // ------------------------------------------------------------------
  // 4. LONG-TERM STORAGE: THINGSPEAK CLOUD DISPATCH (Table 27 @ 60s)
  // ------------------------------------------------------------------
  if (millis() - lastGroupDTick >= groupDInterval) {
    lastGroupDTick = millis();

    // Only attempt a push if the Wi-Fi is actively connected
    if (WiFi.status() == WL_CONNECTED) {
      
      // Strictly map Group D requested parameters to Fields 1-4
      ThingSpeak.setField(1, rpm);
      ThingSpeak.setField(2, pitchAngle);
      ThingSpeak.setField(3, temperature);
      ThingSpeak.setField(4, power);

      ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    }
  }

}