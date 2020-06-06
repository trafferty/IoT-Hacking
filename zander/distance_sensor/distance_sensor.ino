
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
int dlay = 50;

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

  if (inches > 25)
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, HIGH);
    digitalWrite(LEDpin8, HIGH);
    digitalWrite(LEDpin7, HIGH);
    digitalWrite(LEDpin6, HIGH);
  }
  else if ((inches > 20) && (inches <= 25))
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, HIGH);
    digitalWrite(LEDpin8, HIGH); 
    digitalWrite(LEDpin7, HIGH);
    digitalWrite(LEDpin6, LOW);
  }
  else if ((inches > 15) && (inches <= 20))
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, HIGH);
    digitalWrite(LEDpin8, HIGH); 
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin6, LOW);
  }
  else if ((inches > 10) && (inches <= 15))
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, HIGH);
    digitalWrite(LEDpin8, LOW); 
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin6, LOW);
  } 
  else if ((inches > 5) && (inches <= 10))
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, LOW);
    digitalWrite(LEDpin8, LOW); 
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin6, LOW);
    flash = 1;
  } 
  else 
   {
    digitalWrite(LEDpin10, LOW);
    digitalWrite(LEDpin9, LOW);
    digitalWrite(LEDpin8, LOW); 
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin6, LOW);
   }

  if ((1 == flash) && (inches <= 5))
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
