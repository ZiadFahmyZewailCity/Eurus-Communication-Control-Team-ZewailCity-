#include <Arduino.h>

// --- PIN DEFINITIONS ---
#define pin_ENCA 2
#define pin_ENCB 3
#define pin_DIR 8
#define pin_PWM 9

// --- SYSTEM CONSTANTS ---
#define TOTAL_CPR 2206.0 
const float V_rated = 12.0;              // Motor rated voltage
const float Ts = 0.01;                   // 10ms Sampling Time

// --- SIMULINK PD CONTROLLER GAINS ---
// Validated for Angular Displacement (Radians)
const float Kp = 4.101;   
const float Kd = 0.0205;  

// --- CONTROL VARIABLES ---
float target_pos_rads = 0.0;       // Target angular displacement (Rads)
float current_pos_rads = 0.0;      // Current angular displacement (Rads)
float prev_error = 0.0;            // Previous error for derivative

// --- HARDWARE TRACKING VARIABLES ---
volatile long encoderCount = 0; 
unsigned long last_update_time = 0;
unsigned long last_print_time = 0;

// ISR declaration
void updateEncoder();

void setup() {
  Serial.begin(9600);
  
  // Encoder Pins
  pinMode(pin_ENCA, INPUT);
  pinMode(pin_ENCB, INPUT);
  attachInterrupt(digitalPinToInterrupt(pin_ENCA), updateEncoder, RISING);

  // Motor Driver Pins
  pinMode(pin_DIR, OUTPUT);
  pinMode(pin_PWM, OUTPUT);

  // Initial State
  digitalWrite(pin_DIR, LOW);
  analogWrite(pin_PWM, 0);

  Serial.println("====== BRAKING ACTUATOR READY ======");
  Serial.println("Send Target Angular Displacement in RADIANS via Serial Monitor.");
  Serial.println("Example: Send 0.75 for the 15mm equivalent.");
  
  last_update_time = millis();
}

void loop() {
  unsigned long current_time = millis();


  if (current_time - last_update_time >= (Ts * 1000)) {
    last_update_time = current_time;

    // --- A. Atomic Read of Encoder ---
    noInterrupts();
    long safePosition = encoderCount;
    interrupts();

    // --- B. Calculate Current Angular Displacement (Radians) ---
    // Total CPR is 2206. One revolution = 2*PI Radians.
    current_pos_rads = ((float)safePosition / TOTAL_CPR) * 2.0 * PI;

    // --- C. Standard PD Controller (Radians) ---
    float error = target_pos_rads - current_pos_rads;
    
    // Derivative of error 
    float derivative = (error - prev_error) / Ts;
    
    // Control effort: u = Kp * e + Kd * derivative
    float u_control = (Kp * error) + (Kd * derivative);
    
    // Constrain output to valid motor voltage range (-12V to +12V)
    float u_limited = constrain(u_control, -V_rated, V_rated);

    // Determine Direction
    if (u_limited >= 0) {
      digitalWrite(pin_DIR, LOW);  
    } else {
      digitalWrite(pin_DIR, HIGH); 
    }

    // Map constrained voltage (0 to 12V) to PWM (0 to 255)
    // Multiplied by 1000 to maintain integer math precision in the map function
    int pwmValue = map(abs(u_limited) * 1000, 0, V_rated * 1000, 0, 255); 
    pwmValue = constrain(pwmValue, 0, 255);
    analogWrite(pin_PWM, pwmValue);

    // --- E. State Update for Next Iteration ---
    prev_error = error;
  }

  // ==========================================
  // 2. SERIAL INPUT (Target Radians)
  // ==========================================
  if (Serial.available() > 0) {
    float input_rads = Serial.parseFloat();
    
    // Clear buffer
    while(Serial.available() > 0) {
      Serial.read();
    }
      
    target_pos_rads = input_rads;

    Serial.print("\n*** Actuating to: ");
    Serial.print(target_pos_rads, 4);
    Serial.println(" Radians ***\n");
  }

  // ==========================================
  // 3. DEBUG PRINTING (Every 100ms)
  // ==========================================
  if (current_time - last_print_time >= 100) {
    Serial.print("Target: ");
    Serial.print(target_pos_rads, 3);
    Serial.print(" rad | Current: ");
    Serial.print(current_pos_rads, 3);
    Serial.print(" rad | Error: ");
    Serial.println(target_pos_rads - current_pos_rads, 3);
    
    last_print_time = current_time;
  }
}

// --- HARDWARE INTERRUPT ROUTINE ---
void updateEncoder() {
  if (digitalRead(pin_ENCB) == HIGH) {
    encoderCount--; 
  } else {
    encoderCount++; 
  }
}