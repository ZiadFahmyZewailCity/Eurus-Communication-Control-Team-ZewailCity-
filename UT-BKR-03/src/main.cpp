#include <Arduino.h>

#define pin_ENCA 2
#define pin_ENCB 3

#define pin_DIR 8
#define pin_PWM 9

// --- EMPIRICAL MOTOR CONSTANTS ---
// Tested: 6619 ticks for 3 revolutions
#define TOTAL_CPR 2206

// --- BRAKING CALCULATION ---
// Pulley Diameter: 20.5mm -> Circumference: 64.4mm
// Travel needed: 15.0mm -> (15.0 / 64.4) = 0.2329 revolutions
// Target Ticks: 0.2329 * 2206 = ~514 ticks
#define TARGET_TICKS_15MM 514

// volatile is required for variables modified inside an interrupt
volatile long encoderCount = 0; 

// A safe variable to use inside the main loop
long safePosition = 0;

// Timer variable to replace delay()
unsigned long lastPrintTime = 0;

// Function prototype required for strict C++ compilers (like PlatformIO)
void updateEncoder();

void setup() {
  Serial.begin(115200); // Faster baud rate prevents printing from slowing down the motor loop
  
  // Setting interrupt pins 
  // NOTE: This assumes you have PHYSICAL 4.7k pull-up resistors wired to 3.3V!
  // If not, change these to INPUT_PULLUP
  pinMode(pin_ENCA, INPUT);
  pinMode(pin_ENCB, INPUT);

  // Interrupt is triggered on RISING edge (1X decoding)
  attachInterrupt(digitalPinToInterrupt(pin_ENCA), updateEncoder, RISING);

  // DRIVER PINS
  // Setting both pins to output 
  pinMode(pin_DIR, OUTPUT);
  pinMode(pin_PWM, OUTPUT);

  // SET POLARITY FROM THIS VARIABLE (Start with motor off)
  digitalWrite(pin_DIR, LOW);
  analogWrite(pin_PWM, 0);
  
  Serial.println("System Ready. Enter a PWM percentage (0-100):");
}

void loop() {
  int targetPercent = 0;

  // --- FIX 1: THE ATOMIC READ & SAFE COMPARISON ---
  // Safely copy the volatile variable without it changing mid-read
  noInterrupts();
  safePosition = encoderCount;
  interrupts();

  // Always use >= so it catches it even if it overshoots by a single tick!
  if(safePosition >= TOTAL_CPR) {
    analogWrite(pin_PWM, 0);
    Serial.println("Revolved - Motor Stopped!");
  }

  // --- FIX 2: AVOID THE DELAY ---
  // This prints to the screen every 100ms WITHOUT stopping the loop.
  // The loop runs continuously, allowing the brake check above to trigger instantly.
  if (millis() - lastPrintTime >= 100) {
    
    // --- CALCULATE DISPLACEMENTS ---
    // Cast safePosition to a float to prevent integer division from rounding down to zero
    float revs = (float)safePosition / TOTAL_CPR; 
    float angularDisplacement = revs * 360.0;
    float linearDisplacement = revs * 64.4; // 64.4 is the pulley circumference

    // Print all values to the Serial Monitor
    Serial.print("Ticks: ");
    Serial.print(safePosition);
    Serial.print(" | Angle: ");
    Serial.print(angularDisplacement);
    Serial.print(" deg | Linear: ");
    Serial.print(linearDisplacement);
    Serial.println(" mm");
    
    lastPrintTime = millis(); // Reset the stopwatch
  }

  // --- 2. SERIAL COMMAND PARSING ---
  if(Serial.available() > 0) {
    targetPercent = Serial.parseInt();
    
    // Clear rest of the serial buffer 
    while(Serial.available() > 0) {
      Serial.read();
    }

    if (targetPercent >= 0 && targetPercent <= 100) {
      // takes the percentage and turns it to a PWM signal
      uint8_t pwmValue = map(targetPercent, 0, 100, 0, 255);

      Serial.print("Setting Duty Cycle to: ");
      Serial.print(targetPercent);
      Serial.println("%");

      analogWrite(pin_PWM, pwmValue);
    } else {
      Serial.println("Invalid input. Please enter 0 to 100.");
    }
  }
}

// --- INTERRUPT SERVICE ROUTINE ---
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