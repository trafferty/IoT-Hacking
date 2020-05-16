#include <AccelStepper.h>
// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define dirPin 9
#define stepPin 10
#define motorInterfaceType 1
// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

int v_hi = 872;
int v_lo = 130;
int s_hi = 1000;
int s_lo = 100;

int calc_speed(int v_in) {
  return (int)(s_lo + (((v_in - v_lo) / (v_hi -v_lo)) * (s_hi - s_lo)));
}

void setup() {
  Serial.begin( 57600 );

  Serial.println("Configuring Stepper...");

  // Set the maximum speed in steps per second:
  stepper.setMaxSpeed(s_hi);

  stepper.setMinPulseWidth(20);

  // Set the speed in steps per second:
  // 626 gives 188RPM
  // at 1m/s for a 4inch disk
  //stepper.setSpeed(626);
  stepper.setSpeed(400);
 
}

void loop() {

  int sensorValue = analogRead(A0);
  //Serial.println(sensorValue);
  //delay(100);

  //int speed = calc_speed(sensorValue);
  int speed = (sensorValue/10.0) * 10 * 0.3;
  stepper.setSpeed(speed);
  Serial.println(speed);
  // Step the motor with a constant speed as set by setSpeed():
  stepper.runSpeed();  
  
  
  
}
