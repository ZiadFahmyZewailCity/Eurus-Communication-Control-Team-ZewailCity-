#include <Arduino.h>

// Pins
#define pin_ENCA 2
#define pin_ENCB 3
#define pin_DIR 8
#define pin_PWM 9

// System parameters
#define TOTAL_CPR 2206.0 
const float V_rated = 12.0;              
const float Ts = 0.01;                   
const unsigned long Ts_ms = 10; // Integer timer for embedded performance

// PID Gains (Derived from Simulink for angular displacement)
const float Kp = 4.101;   
const float Kd = 0.0205;  
const float Ki = 1.5;     // NEW: Conditional Integrator Gain (Tune as needed)

// Control Variables
float target_pos_rads = 0.0;       
float current_pos_rads = 0.0;      
float prev_error = 0.0;            
float integral_sum = 0.0;          // NEW: Accumulator for the temporary integrator

// Hardware Variables
volatile long encoderCount = 0; 
unsigned long last_update_time = 0;

// Function Declarations
void updateEncoder();

// Prevents overwrites by disabling interrupts during read 
long atomic_encoderRead() {
  noInterrupts();
  long sampledCount = encoderCount;
  interrupts(); 
  return sampledCount;
}

// Angular displacement given total CPR and counts
float angularDisplacement(long counts_revolution, float totalCPR) {
  return ((float)counts_revolution / totalCPR) * 2.0 * PI;
}

// Simple error (target - actual)
float error(float target, float actual) {
  return target - actual;
}

// Derivative according to predefined time step
float currentDerivative(float current, float previous) {
   return (current - previous) / Ts;
}

// UPDATED: Control effort calculation now includes Integral term
float controller_PID(float error, float derivative, float integral, float gain_p, float gain_d, float gain_i) {
  return (gain_p * error) + (gain_d * derivative) + (gain_i * integral);
}

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

  // Time tracking
  last_update_time = millis();
  
  Serial.println("====== BRAKING ACTUATOR READY ======");
}

void loop() {
  unsigned long current_time = millis();

  // ==========================================
  // 1. 10ms CONTROL LOOP (Ts = 0.01s)
  // ==========================================
  if (current_time - last_update_time >= Ts_ms) {
    last_update_time = current_time;
    
    // Math & Kinematics
    float current_angularDis = angularDisplacement(atomic_encoderRead(), TOTAL_CPR);
    float current_error = error(target_pos_rads, current_angularDis);
    float derivative = currentDerivative(current_error, prev_error);

    // --- CONDITIONAL INTEGRATOR LOGIC ---
    // Only integrate if stuck close to the target (between 0.02 and 0.2 rads)
    if (abs(current_error) < 0.2 && abs(current_error) > 0.02) {
        integral_sum += current_error * Ts;
    } else {
        // Reset if making a large movement OR if target is safely reached
        integral_sum = 0.0;
    }

    // Calculate full PID output
    float pid_output = controller_PID(current_error, derivative, integral_sum, Kp, Kd, Ki);
    float limited_pid_output = constrain(pid_output, -V_rated, V_rated);
  
    // Determine Direction
    if (pid_output >= 0) {
      digitalWrite(pin_DIR, LOW);  
    } else {
      digitalWrite(pin_DIR, HIGH); 
    }

    // Map constrained voltage (0 to 12V) to PWM (0 to 255)
    int pwmValue = map(abs(limited_pid_output) * 1000, 0, V_rated * 1000, 0, 255); 

    // --- STICTION FEEDFORWARD & DEADBAND ---
    int MIN_PWM = 20; 
    
    // Feedforward: Provide baseline power when controller is trying to move
    if (pwmValue > 0 && pwmValue < MIN_PWM) {
        pwmValue = MIN_PWM; 
    }

    // Deadband: Stop motor completely if within acceptable tolerance (~0.02 rads)
    if (abs(current_error) <= 0.02) {
        pwmValue = 0;
    }

    // Output signal
    pwmValue = constrain(pwmValue, 0, 255);
    analogWrite(pin_PWM, pwmValue);

    // Update error for next iteration
    prev_error = current_error;

    // --- DEBUG PRINTING ---
    static int printCounter = 0;
    printCounter++;
    
    // Prints every 10th loop (100ms) to prevent Serial lag
    if (printCounter >= 10) { 
      Serial.print("Target:");
      Serial.print(target_pos_rads, 3);
      Serial.print(",Position:");
      Serial.print(current_angularDis, 3);
      Serial.print(",Error:");
      Serial.println(current_error, 3);
      
      printCounter = 0;
    }
  } // <--- End of 10ms timer block

  // ==========================================
  // 2. SERIAL COMMUNICATION
  // ==========================================
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() > 0) {
      // CRITICAL: Wipe the integrator clean when a new command is received
      integral_sum = 0.0; 

      if (input.equalsIgnoreCase("r")) {
        target_pos_rads = 0.0;
        Serial.println("\n-> Returning to physical zero point...\n");
      } else {
        target_pos_rads = input.toFloat();
        Serial.print("\n-> Target set to: ");
        Serial.print(target_pos_rads);
        Serial.println(" rads\n");
      }
    }
  }
}

// ISR for encoder counting
void updateEncoder() {
  if (digitalRead(pin_ENCB) == HIGH) {
    encoderCount--; 
  } else {
    encoderCount++; 
  }
}