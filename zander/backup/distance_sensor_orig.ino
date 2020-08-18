
int trigPin = 11;    // Trigger
int echoPin = 12;    // Echo
int LEDpin10 = 10;    // LED output
int LEDpin9 = 9;
int LEDpin8 = 8;
int LEDpin7 = 7;
int LEDpin6 = 6;
long inches;
int delay_ms = 200;
int flash = 1;
//int flosh = 1;
int dlay = 40;
int dlay_start = 0;
int current_ms = millis();
int s_unknown = 0;
int state = s_unknown;

void set_lights(int pin6, int pin7, int pin8, int pin9, bool pin10)
{
    digitalWrite(LEDpin10, pin10);
    digitalWrite(LEDpin9, pin9);
    digitalWrite(LEDpin8, pin8);
    digitalWrite(LEDpin7, pin7);
    digitalWrite(LEDpin6, pin6);
}


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

  inches = 25;
}

// the loop function runs over and over again forever
void loop() {
  
  float duration, cm;
  
  current_ms = millis();
  //Serial.print(current_ms);
  
  if (inches > 105)
  {
    set_lights(LOW, LOW, LOW, LOW, LOW);
  }

  else if ((inches > 85) && (inches <= 102))
  {
    set_lights(HIGH, HIGH, HIGH, HIGH, HIGH);
  }
  else if ((inches > 68) && (inches <= 85))
  {
    set_lights(LOW, HIGH, HIGH, HIGH, HIGH);
  }
  else if ((inches > 51) && (inches <= 68))
  {
    set_lights(LOW, LOW, HIGH, HIGH, HIGH);
  }
  else if ((inches > 34) && (inches <= 51))
  {
    set_lights(LOW, LOW, LOW, HIGH, HIGH);
  } 
  else if ((inches > 18) && (inches <= 34))
  {
    set_lights(LOW, LOW, LOW, LOW, HIGH);
    flash = 1;
  } 
  else 
   {
    set_lights(LOW, LOW, LOW, LOW, LOW);
   }

  if ((1 == flash) && (inches <= 18))
   {
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