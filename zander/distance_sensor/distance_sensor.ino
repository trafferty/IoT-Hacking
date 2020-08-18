
int trigPin = 11;    // Trigger
int echoPin = 12;    // Echo
int LEDpin10 = 10;    // LED output
int LEDpin9 = 9;
int LEDpin8 = 8;
int LEDpin7 = 7;
int LEDpin6 = 6;
int testPin = 13;
long inches;
int delay_ms = 200;
int flash = 1;
//int flosh = 1;
int dlay = 40;
int dlay_start = 0;
int current_ms = millis();
int s_unknown = 0;
int state = s_unknown;
int D6 = 999;
int D5 = 999;
int D4 = 999;
int D3 = 999;
int D2 = 999;
int D1 = 999;

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
    D5 = 10;
    D4 = 8;
    D3 = 6;
    D2 = 4;
    D1 = 2;
  } else {
    D6 = 105;
    D5 = 85;
    D4 = 68;
    D3 = 51;
    D2 = 34;
    D1 = 18;
  }

  inches = 25;
}

// the loop function runs over and over again forever
void loop() {
  
  float duration, cm;
  
  current_ms = millis();
  //Serial.print(current_ms);
  
  if (inches > D6)
  {
    set_lights(LOW, LOW, LOW, LOW, LOW);
  }
  else if ((inches > D5) && (inches <= D6))
  {
    set_lights(HIGH, HIGH, HIGH, HIGH, HIGH);
  }
  else if ((inches > D4) && (inches <= D5))
  {
    set_lights(LOW, HIGH, HIGH, HIGH, HIGH);
  }
  else if ((inches > D3) && (inches <= D4))
  {
    set_lights(LOW, LOW, HIGH, HIGH, HIGH);
  }
  else if ((inches > D2) && (inches <= D3))
  {
    set_lights(LOW, LOW, LOW, HIGH, HIGH);
  } 
  else if ((inches > D1) && (inches <= D2))
  {
    set_lights(LOW, LOW, LOW, LOW, HIGH);
    flash = 1;
  } 
  else 
   {
    set_lights(LOW, LOW, LOW, LOW, LOW);
   }

  if ((1 == flash) && (inches <= D1))
   {
    do_flash(dlay);
    do_flash(dlay);

    flash = 2;
   }


  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
  
  Serial.println(inches);

  delay(50);
}

void set_lights(int pin6, int pin7, int pin8, int pin9, bool pin10)
{
    digitalWrite(LEDpin10, pin10);
    digitalWrite(LEDpin9, pin9);
    digitalWrite(LEDpin8, pin8);
    digitalWrite(LEDpin7, pin7);
    digitalWrite(LEDpin6, pin6);
}
void do_flash(int dlay) {
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
