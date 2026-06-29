#pragma once

constexpr float VOLT_SENS_VREF = 5.0f;

// --- LOCKED HARDWARE DIVIDER: 47k (Top) into 2.2k (Bottom) ---
constexpr float R1_KOHM = 47.0f;
constexpr float R2_KOHM = 2.2f;

// The compiler solves this equation on your PC during compilation. 
// Ratio = (47 + 2.2) / 2.2 = 22.363636f
constexpr float VOLTAGE_DIVIDER_RATIO = (R1_KOHM + R2_KOHM) / R2_KOHM;

// Fine-tune software trim here once you test it against a multimeter
constexpr float VOLT_CAL_FACTOR = 1.0f;