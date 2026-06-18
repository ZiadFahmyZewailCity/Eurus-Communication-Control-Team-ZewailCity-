#include <Arduino.h>
#include <WiFi.h>
#include <ThingSpeak.h>

//Wifi Credidentials
const char* ssid = "ZC-Guest";
const char* password = "rB%4159-o.hazS";

WiFiClient me;

//API KEY
const char* writeAPIKey = "I05J5XNSOSM1IQDQ";
uint32_t ID_channel = 3407005;

//Channel IDs
#define ID_CHANNEL_RPM 1
#define ID_CHANNEL_PITCH 2
#define ID_CHANNEL_POWER 3
#define ID_CHANNEL_TEMPERATURE 4

//Channel Values
int PITCH = 0;
int OUTPUT_POWER = 0;
int RPM = 0;
int TEMPERATURE = 0;

//Delay for timer
unsigned long previousTransmissionTime = 0;
unsigned long transmissionInterval = 15000;

//Number of samples taken
long long int count_aggregated = 0;

//Functions takes the ssid & pass word and attempts to connect you to the internet
bool connectingToWifi(const char* ssid, const char* pass)
{
    if(WiFi.status() != WL_CONNECTED)
    {
        return false;
    }   
    else 
    {
        return true;
    }
}

void setup() {
    //Connecting to wifi 
    Serial.begin(115200);

    Serial.print("Connecting to wifi");
    WiFi.begin(ssid,password);
    while(!connectingToWifi(ssid,password))
    {
        Serial.print(".");
        delay(500);
    }
    Serial.print("\nSuccessfully connected\n");

    ThingSpeak.begin(me);
}

void loop() {
    //Blocking Delay to reduce amount of summed variables 
    delay(500);

    //Generate random values each loop to figure out what 
    TEMPERATURE += random(0,90);
    PITCH += random(0,70);
    OUTPUT_POWER += random(0,1000);
    RPM += random(0,600);
    count_aggregated++;

    unsigned long currentTime = millis();

    if(transmissionInterval <= (currentTime - previousTransmissionTime))
    {
        previousTransmissionTime = currentTime;
        
        float average_temp = (float)TEMPERATURE/(float)count_aggregated;
        float average_pitch = (float)PITCH/(float)count_aggregated;
        float average_power = (float)OUTPUT_POWER/(float)count_aggregated;
        float average_RPM = (float)RPM/(float)count_aggregated;

        ThingSpeak.setField(ID_CHANNEL_TEMPERATURE,average_temp);
        ThingSpeak.setField(ID_CHANNEL_PITCH,average_pitch);
        ThingSpeak.setField(ID_CHANNEL_POWER,average_power);
        ThingSpeak.setField(ID_CHANNEL_RPM,average_RPM);

        int HTTP_speakResponse = ThingSpeak.writeFields(ID_channel,writeAPIKey);

        if(HTTP_speakResponse == 200)
        {
            Serial.print("Sent Succefully\n");
            Serial.printf("Sent Data -> RPM: %.2f | Temp: %.2f | Pitch: %.2f | Power: %.2f\n", average_RPM, average_temp, average_pitch, average_power);
        }
        else
        {
            Serial.print("Failed to send\n");
        }

        TEMPERATURE = 0;
        PITCH = 0;
        OUTPUT_POWER = 0;
        RPM = 0;
        count_aggregated = 0;
    }
}