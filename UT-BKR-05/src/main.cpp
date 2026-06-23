#include <Arduino.h>

#define BRAKE_INT_PIN 2

volatile bool breakState = false;
volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // 50ms debounce time

// Forward declarations of ISRs
void brake_rising();
void brake_falling();

void setup() {
  Serial.begin(9600); // Don't forget to initialize Serial!
  
  pinMode(BRAKE_INT_PIN, INPUT); 
  
  attachInterrupt(digitalPinToInterrupt(BRAKE_INT_PIN), brake_rising, FALLING);
}

void loop() {
  Serial.print("State: ");
  Serial.println(breakState);
  delay(100); // Keeps the serial monitor readable
}

void brake_rising() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay) {
    breakState = true;
    lastDebounceTime = currentTime;
    // Next, listen for the FALLING edge
    attachInterrupt(digitalPinToInterrupt(BRAKE_INT_PIN), brake_falling, RISING);
  }
}

void brake_falling() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay) {
    breakState = false;
    lastDebounceTime = currentTime;
    // Next, listen for the RISING edge
    attachInterrupt(digitalPinToInterrupt(BRAKE_INT_PIN), brake_rising, FALLING);
  }
}