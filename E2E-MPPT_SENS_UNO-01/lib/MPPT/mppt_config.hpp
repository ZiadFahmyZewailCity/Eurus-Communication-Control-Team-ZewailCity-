// config.hpp
#pragma once // Prevents this file from being included twice by the compiler

// --- PINS ---
#define ESP_RX_PIN 2
#define ESP_TX_PIN 3
#define RXD2 16
#define TXD2 17

// --- TIMING CONSTANTS ---
//Recall to decided sampl_internval accuratly later
constexpr unsigned long SAMPLE_INTERVAL = 250;  
constexpr unsigned long REPORT_INTERVAL = 500;

// --- MPPT TUNING ---
//Double check the step size
constexpr float MPPT_STEP = 0.005f;
constexpr float MPPT_MIN_DUTY = 0.15f;
constexpr float MPPT_MAX_DUTY = 0.90f;