#pragma once
#include <Arduino.h>

//Packed attribute makes it so compiler doesnt by default add padding bits
//Padding bits would ruin the determinism of our communicaito
struct __attribute__((packed)) internal_payLoad {

//These bytes are used to make sure communication is synced
//You set this to a known value by both sides of the communication
//Its essentially just a header to indicate this is the start of the byte
uint8_t syncByte_1;
uint8_t syncByte_2;

uint8_t brakeACK;

float rpm;
float power;
float currentConv;
float voltageConv;
float currentRect;
float voltageRect;
float pitchAngle;
float temperature;

};