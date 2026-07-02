#pragma once
#include <Arduino.h>

//RPM Encoder pins
constexpr uint8_t RPM_ENCODER_PIN_A = 2;   
constexpr uint8_t RPM_ENCODER_PIN_B = 3;

//Braking Motor Pins
constexpr uint8_t pin_ENCA = 18;
constexpr uint8_t pin_ENCB = 19;
constexpr uint8_t pin_DIR  = 22;
constexpr uint8_t pin_PWM  = 8;

//MPPT
constexpr uint8_t MPPT_PIN = 6;
//Current Sensor pins
constexpr uint8_t pre_converter_pin  = A0;
constexpr uint8_t post_converter_pin = A1;
//Rectifier voltage divider pin
constexpr uint8_t voltageDividerPin = A2;
//Converter Differential Voltage pins
constexpr uint8_t differentialVoltage_VA_PIN = A3;
constexpr uint8_t differentialVoltage_VB_PIN = A5;

//Temperature pins
constexpr uint8_t temperature_1_pin = 36; 
constexpr uint8_t temperature_2_pin = 37;
//ESP32 
constexpr uint8_t ESP32_RX = 17;
constexpr uint8_t ESP32_TX = 16;
