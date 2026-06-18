#include <Arduino.h>

#define pin_ENCA 2
#define pin_ENCB 3

#define pin_DIR 8
#define pin_PWM 9

//Based on a test
#define TOTAL_CPR 2206 

uint8_t pwmValue = 0;

void updateEncoder();

// volatile is required for variables modified inside an interrupt
volatile long encoderCount = 0; 
long safePosition = 0;
bool brakeEngaged = false;
unsigned long lastPrintTime = 0; 

void setup() {
  Serial.begin(9600); 
  
  // Setting interrupt pins
  // Added PULLUP to keep your signal clean
  pinMode(pin_ENCA, INPUT);
  pinMode(pin_ENCB, INPUT);

  // Interrupt is triggered on RISING
  attachInterrupt(digitalPinToInterrupt(pin_ENCA), updateEncoder, RISING);

  // DRIVER PINS
  // Setting both pins to output 
  pinMode(pin_DIR, OUTPUT);
  pinMode(pin_PWM, OUTPUT);

  // SET POLARITY FROM THIS VARIABLE
  digitalWrite(pin_DIR, LOW);
  analogWrite(pin_PWM, 0);
}

void loop() 
{
  int targetPercent = 0;

  // --- 1. ATOMIC READ ---
  // Safely grab the count so we don't read a corrupted number
  noInterrupts();
  safePosition = encoderCount;
  interrupts();

  // --- 2. INSTANT BRAKING ---
  // Use >= so it NEVER misses the stop, even if it overshoots by a tick
  if(safePosition >= TOTAL_CPR && !brakeEngaged)
  {
    pwmValue = 0;
    analogWrite(pin_PWM, 0); // Actually turn off the motor pin!
    Serial.println("\n*** Revolved - Motor Stopped! ***\n");
    brakeEngaged = true; // Prevents it from spamming the serial monitor and crashing
  }

  // --- 3. MATH & PRINTING (NO DELAYS) ---
  // This prints every 100ms, allowing the loop to run instantly to catch the brake
  if (millis() - lastPrintTime >= 100) {
    
    // Math Calculations
    float revs = (float)safePosition / TOTAL_CPR; 
    float angularDisplacement = revs * 360.0;
    float linearDisplacement = revs * 64.4; // 64.4mm is the pulley circumference

    Serial.print("Position: ");
    Serial.print(safePosition);
    Serial.print(" | Angle: ");
    Serial.print(angularDisplacement);
    Serial.print(" deg | Linear: ");
    Serial.print(linearDisplacement);
    Serial.println(" mm");
    
    lastPrintTime = millis();
  }

  // --- 4. SERIAL INPUT ---
  if(Serial.available() > 0){

    // Clear rest of the serial buffer 
    targetPercent = Serial.parseInt();
    while(Serial.available() > 0){
      Serial.read();
    }
      
    if (targetPercent >= 0 && targetPercent <= 100)
    {
      // takes the percentage and turns it to a PWM signal
      pwmValue = map(targetPercent, 0, 100, 0, 255);

      Serial.print("% Duty Cycle: ");
      Serial.println(targetPercent);

      // Output signal
      analogWrite(pin_PWM, pwmValue);
      brakeEngaged = false; // Reset the brake so we can spin again

    }
    else
    {
      Serial.println("Invalid input");
    }
  }
}

// ISR
void updateEncoder() {
  // Read the state of Phase B to determine direction
  if (digitalRead(pin_ENCB) == HIGH) {
    // Forward
    encoderCount++; 
  } else {
    // Backward
    encoderCount--; 
  }
}