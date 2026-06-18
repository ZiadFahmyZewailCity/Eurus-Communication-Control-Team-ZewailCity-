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
int previousTransmissionTime = 0;
unsigned long int transmissionInterval = 15000; // 15 seconds in milliseconds

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
        //The wifi will take a bit of time trying to connect to the router
        Serial.print(".");
        //Dont use delays in the final product
        delay(500);
    }
    Serial.print("Successfully connected\n");

    ThingSpeak.begin(me);


}

void loop() {

    //Generate random values each loop to figure out what 
    
    TEMPERATURE += random(0,90);
    PITCH += random(0,70);
    OUTPUT_POWER += random(0,1000);
    RPM += random(0,600);
    count_aggregated++;


    //DO NOT USE THIS APPROACH IN THE REAL SYSTEM AS THIS LIMITS RUNNING TILL OVERFLOW
    int currentTime = millis();

    if(transmissionInterval <= (currentTime - transmissionInterval))
    {

        float average_temp = TEMPERATURE/count_aggregated;
        float  average_pitch = PITCH/count_aggregated;
        float average_power = OUTPUT_POWER/count_aggregated;
        float average_RPM = RPM/count_aggregated;

        ThingSpeak.setField(ID_CHANNEL_TEMPERATURE,average_temp);
        ThingSpeak.setField(ID_CHANNEL_PITCH,average_pitch);
        ThingSpeak.setField(ID_CHANNEL_POWER,average_power);
        ThingSpeak.setField(ID_CHANNEL_RPM,average_RPM);

        int HTTP_speakResponse = ThingSpeak.writeFields(ID_channel,writeAPIKey);

        if(HTTP_speakResponse == 200)
        {
            Serial.print("Sent Succefully\n");
            Serial.printf("Sent Data -> RPM: %d | Temp: %d | Pitch: %d | Power: %d\n", RPM, TEMPERATURE, PITCH, OUTPUT_POWER);
        }
        else
        {
            Serial.print("Failed to send\n");
        }

    }



}


