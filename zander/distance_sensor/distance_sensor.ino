
enum CarStates_t {  S_UNKNOWN=0, S_NO_CAR, S_CAR_D5, S_CAR_D4, S_CAR_D3, 
                    S_CAR_D2, S_CAR_D1, S_CAR_PERFECT, S_TOO_CLOSE };

int trigPin = 11;    // Trigger
int echoPin = 12;    // Echo
int LEDpin10 = 10;    // LED output
int LEDpin9 = 9;
int LEDpin8 = 8;
int LEDpin7 = 7;
int LEDpin6 = 6;
int testPin = 13;
int burst = 0;
float distance_inches;
int delay_ms = 200;
int dlay = 40;
int dlay_start = 0;
long state_change_ts_ms;
int time_out_ms = 15000;
bool timed_out = false;
CarStates_t previous_state = S_UNKNOWN;
int D6 = 999;
int D5 = 999;
int D4 = 999;
int D3 = 999;
int D2 = 999;
int D1 = 999;
int D0 = 999;
bool test_mode = false;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LEDpin10, OUTPUT);
    //Serial Port begin
  Serial.begin (57000);
  
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDpin10, OUTPUT);
  pinMode(LEDpin9, OUTPUT);
  pinMode(LEDpin8, OUTPUT);
  pinMode(LEDpin7, OUTPUT);
  pinMode(LEDpin6, OUTPUT);
  pinMode(testPin, INPUT_PULLUP);

  if (digitalRead(testPin) == HIGH) {
    test_mode = true;
    Serial.println("**** Warning: In test Mode! *****");
  }

  if (test_mode) {
    D6 = 12;
    D5 = 11;
    D4 = 9;
    D3 = 7;
    D2 = 5;
    D1 = 3;
    D0 = 2;
  } else {
    D6 = 100;  // beyond this distance, no car
    D5 = 80;   // 
    D4 = 66;
    D3 = 40;  // 51
    D2 = 30;  // 34
    D1 = 18.5;   //18 car at perfect distance
    D0 = 13.5;   //15 closest car can be!
  }

  distance_inches = 999;
}

// the loop function runs over and over again forever
void loop() {
 
  CarStates_t new_state = S_UNKNOWN;

  distance_inches = get_distance();

  /*
   * First, check which region the distance falls in
   * and set the proper state.
   */
  if (distance_inches > D6)
  {
    new_state = S_NO_CAR;
  }
  else if ((distance_inches > D5) && (distance_inches <= D6))
  {
    new_state = S_CAR_D5;
  }
  else if ((distance_inches > D4) && (distance_inches <= D5))
  {
    new_state = S_CAR_D4;
  }
  else if ((distance_inches > D3) && (distance_inches <= D4))
  {
    new_state = S_CAR_D3;
  }
  else if ((distance_inches > D2) && (distance_inches <= D3))
  {
    new_state = S_CAR_D2;
  } 
  else if ((distance_inches > D1) && (distance_inches <= D2))
  {
    new_state = S_CAR_D1;
  } 
  else if ((distance_inches > D0) && (distance_inches <= D1))
  {
    new_state = S_CAR_PERFECT;
  } 
  else if (distance_inches <= D0)
  {
    new_state = S_TOO_CLOSE;
  } 

  /* Now do some action based on the new state.  If the new state
   *  has not changed from previous loop, just check for timeout
   */
  if (new_state == S_TOO_CLOSE)
  {
    Serial.println("Car TOO CLOSE!");
    previous_state = new_state;
    do_flash(100);
  }
  else if (new_state != previous_state)
  {
    previous_state = new_state;
    state_change_ts_ms = millis();
    timed_out = false;
    
    Serial.println(distance_inches);
 
    switch (new_state)
    {
      case S_NO_CAR:
        Serial.println("No Car!");
        set_lights(LOW, LOW, LOW, LOW, LOW);
        break;
        
      case S_CAR_D5:
        Serial.println("Car at D5");
        set_lights(HIGH, HIGH, HIGH, HIGH, HIGH);
        break;
        
      case S_CAR_D4:
        Serial.println("Car at D4");
        set_lights(LOW, HIGH, HIGH, HIGH, HIGH);
        break;
        
      case S_CAR_D3: 
        Serial.println("Car at D3");
        set_lights(LOW, LOW, HIGH, HIGH, HIGH);
        break;
        
      case S_CAR_D2:
        Serial.println("Car at D2");
        set_lights(LOW, LOW, LOW, HIGH, HIGH);
        break;
        
      case S_CAR_D1:
        Serial.println("Car at D1");
        set_lights(LOW, LOW, LOW, LOW, HIGH);
        break;
        
      case S_CAR_PERFECT:
        Serial.println("Car perfect!");
        do_burst(dlay);
        do_burst(dlay);
        break;
     }
  }
  else if (new_state == previous_state)
  {
    long delta = millis() - state_change_ts_ms;
    if (( delta > time_out_ms) && !timed_out)
    {
      Serial.println(delta);
      set_lights(LOW, LOW, LOW, LOW, LOW);
      timed_out = true;
    }
  }
    
  delay(50);
}

float get_distance()
{
  float distance_inches;
  long distance_cm;
  float duration;
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  distance_cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  distance_inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135

  return distance_inches;
}

void set_lights(int pin6, int pin7, int pin8, int pin9, bool pin10)
{
    digitalWrite(LEDpin10, pin10);
    digitalWrite(LEDpin9, pin9);
    digitalWrite(LEDpin8, pin8);
    digitalWrite(LEDpin7, pin7);
    digitalWrite(LEDpin6, pin6);
}
void do_flash(int dlay)
{
  set_lights(HIGH, HIGH, HIGH, HIGH, HIGH);
  delay(dlay);
  set_lights(LOW, LOW, LOW, LOW, LOW);
  delay(dlay);
//  set_lights(HIGH, HIGH, HIGH, HIGH, HIGH);
//  delay(dlay);
//  set_lights(LOW, LOW, LOW, LOW, LOW);
//  delay(dlay);
//  set_lights(HIGH, HIGH, HIGH, HIGH, HIGH);
//  delay(dlay);
//  set_lights(LOW, LOW, LOW, LOW, LOW);
}

void do_burst(int dlay) {
    digitalWrite(LEDpin10, HIGH);
    delay(dlay);
    digitalWrite(LEDpin10, LOW);
    digitalWrite(LEDpin9, HIGH);
    delay(dlay);
    digitalWrite(LEDpin9, LOW);
    digitalWrite(LEDpin8, HIGH);
    digitalWrite(LEDpin10, HIGH);
    delay(dlay);
    digitalWrite(LEDpin8, LOW);
    digitalWrite(LEDpin10, LOW);
    digitalWrite(LEDpin7, HIGH);
    digitalWrite(LEDpin9, HIGH);
    delay(dlay);
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin9, LOW);
    digitalWrite(LEDpin6, HIGH);
    digitalWrite(LEDpin8, HIGH);
    delay(dlay);
    digitalWrite(LEDpin6, LOW);
    digitalWrite(LEDpin8, LOW);
    digitalWrite(LEDpin7, HIGH);
    delay(dlay);
    digitalWrite(LEDpin7, LOW);
    
    digitalWrite(LEDpin8, HIGH);
    digitalWrite(LEDpin6, HIGH);
    delay(dlay);
    digitalWrite(LEDpin8, LOW);
    digitalWrite(LEDpin6, LOW);
    digitalWrite(LEDpin9, HIGH);
    digitalWrite(LEDpin7, HIGH);
    delay(dlay);
    digitalWrite(LEDpin9, LOW);
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin10, HIGH);
    delay(dlay);
    digitalWrite(LEDpin10, LOW);
}
