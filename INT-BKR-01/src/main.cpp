#include <Arduino.h>

//Pins
#define pin_ENCA 2
#define pin_ENCB 3
#define pin_DIR 8
#define pin_PWM 9

//System parameters
#define TOTAL_CPR 2206.0 
const float V_rated = 12.0;              
const float Ts = 0.01;                   

//PD Gains
const float Kp = 4.101;   
const float Kd = 0.0205;  
//Control Variables
float target_pos_rads = 0.0;       
float current_pos_rads = 0.0;      
float prev_error = 0.0;            
//Hardware Variables
volatile long encoderCount = 0; 
unsigned long last_update_time = 0;
unsigned long last_print_time = 0;


//Update encoder decleration
void updateEncoder();



//Prevents over writes by disabling interrupts during read 
//Reads voltaile encoder value directly
long atomic_encoderRead()
{
  noInterrupts();
  long sampledCount = encoderCount;
  interrupts(); 
  return sampledCount;
}

//Angular displacment given total CPR and counts
float angularDisplacement(long counts_revolution, float totalCPR)
{
  //Current Angular Displacment
  return ((float)counts_revolution / totalCPR) * 2.0 * PI;

}

//Simple error (target - actual)
float error(float target, float actual)
{
  return target - actual;
}

//Derivative according to predefined time step
float currentDerivative(float current, float previous)
{
   return (current - previous)/Ts;
}

float controller_PD(float error, float derivative, float gain_proportional, float gain_derivative)
{
  return (gain_proportional * error) + (gain_derivative * derivative);
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

  //Time tracking
  last_update_time = millis();

}

void loop() {


  unsigned long current_time = millis();

  if (current_time - last_update_time >= (Ts * 1000)) {
    last_update_time = current_time;
    

    float current_angularDis = angularDisplacement(atomic_encoderRead(),TOTAL_CPR);
    
    float current_error = error(target_pos_rads,current_angularDis);
    
    float derivative = currentDerivative(current_error,prev_error);
    
    float pd_output = controller_PD(current_error,derivative,Kp,Kd);
    
    float limited_pd_output = constrain(pd_output, -V_rated, V_rated);
  
    // Determine Direction
    if (pd_output >= 0) {
      digitalWrite(pin_DIR, LOW);  
    } else {
      digitalWrite(pin_DIR, HIGH); 
    }

    // Map constrained voltage (0 to 12V) to PWM (0 to 255)
    // Multiplied by 1000 to maintain integer math precision in the map function
    int pwmValue = map(abs(limited_pd_output) * 1000, 0, V_rated * 1000, 0, 255); 
    pwmValue = constrain(pwmValue, 0, 255);
    analogWrite(pin_PWM, pwmValue);

    //Update error
    prev_error = current_error;

  }

  // ==========================================
  // 2. SERIAL COMMUNICATION
  // ==========================================
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() > 0) {
      if (input.equalsIgnoreCase("r")) {
        
        // Let the control system drive it back to the original starting point
        target_pos_rads = 0.0;
        
        Serial.println("-> Returning to physical zero point...");
      } else {
        target_pos_rads = input.toFloat();
        Serial.print("-> Target set to: ");
        Serial.println(target_pos_rads);
      }
    }
  }




}

//ISR for encoder counting
void updateEncoder() {
  if (digitalRead(pin_ENCB) == HIGH) {
    encoderCount--; 
  } else {
    encoderCount++; 
  }
}