
int LEDpin10 = 10;    // LED output
int LEDpin9 = 9;
int LEDpin8 = 8;
int LEDpin7 = 7;
int LEDpin6 = 6;
int delay_ms = 200;
int dlay = 50;
int state = 0;

// the setup function runs once when you press reset or power the board
void setup() {
    //Serial Port begin
  Serial.begin (57000);
  
  pinMode(LEDpin10, OUTPUT);
  pinMode(LEDpin9, OUTPUT);
  pinMode(LEDpin8, OUTPUT);
  pinMode(LEDpin7, OUTPUT);
  pinMode(LEDpin6, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {

  if (6 == state) {
    state = 0;
  }
  
  if (0 == state)
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, HIGH);
    digitalWrite(LEDpin8, HIGH);
    digitalWrite(LEDpin7, HIGH);
    digitalWrite(LEDpin6, HIGH);
  }
  else if (1 == state)
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, HIGH);
    digitalWrite(LEDpin8, HIGH); 
    digitalWrite(LEDpin7, HIGH);
    digitalWrite(LEDpin6, LOW);
  }
  else if (2 == state)
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, HIGH);
    digitalWrite(LEDpin8, HIGH); 
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin6, LOW);
  }
  else if (3 == state)
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, HIGH);
    digitalWrite(LEDpin8, LOW); 
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin6, LOW);
  } 
  else if (4 == state)
  {
    digitalWrite(LEDpin10, HIGH);
    digitalWrite(LEDpin9, LOW);
    digitalWrite(LEDpin8, LOW); 
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin6, LOW);
  } 
  else
  {
    digitalWrite(LEDpin10, LOW);
    digitalWrite(LEDpin9, LOW);
    digitalWrite(LEDpin8, LOW); 
    digitalWrite(LEDpin7, LOW);
    digitalWrite(LEDpin6, LOW);
  } 

  //state += 1;
  state =0;

  delay(50);
}
