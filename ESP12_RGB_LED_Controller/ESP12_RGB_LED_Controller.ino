
// For reference (pin mapping for ESP8266-12E):
// D0/GPIO16      = 16;
// D1/GPIO5       = 5;
// D2/GPIO4       = 4;
// D3/GPIO0       = 0;

// D4/GPIO2       = 2;

// D5/GPIO14      = 14;
// D6/GPIO12      = 12;
// D7/GPIO13      = 13;
// D8/GPIO15      = 15;

// Pin mapping for the first string of lights. Pins D5 - D7
#define BLUE_LED_OUT      15
#define RED_LED_OUT       12
#define GREEN_LED_OUT     14

#define MOTION_DETECTED_LED  16
#define PIR_IN  5

#define FADESPEED 3     // make this higher to slow down

typedef enum {
    blue_to_violet, violet_to_red, red_to_yellow, yellow_to_green, green_to_teal, teal_to_blue
} LED_Fade_States_t;

const int maxBrightness = 1024;
//int maxBrightness = 500;
const int minBrightness = 0;

// Status variables
// Temporary holding for current values
int currentR = 0;
int currentG = 0;
int currentB = 0;

LED_Fade_States_t fade_state;

void setup() 
{
  Serial.begin( 57600 );
  delay( 100 );

  // Prepare the pins to fire
  pinMode( BLUE_LED_OUT, OUTPUT );
  pinMode( RED_LED_OUT, OUTPUT );
  pinMode( GREEN_LED_OUT, OUTPUT );
  pinMode( MOTION_DETECTED_LED, OUTPUT );

  // turn all the lights on when the ESP first powers on
  turnOn( );
  delay(500);
  turnOff();

  digitalWrite(MOTION_DETECTED_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(MOTION_DETECTED_LED, LOW);    // turn the LED off by making the voltage LOW
  delay(200);                       // wait for a second

  currentR = minBrightness;
  currentG = minBrightness;
  currentB = maxBrightness;
  fade_state = blue_to_violet;
}

void fadeLEDs()
{
  switch (fade_state)
  {
    case blue_to_violet:
      currentR += 1;
      analogWrite(RED_LED_OUT, currentR);
      if (currentR == maxBrightness)
        fade_state = violet_to_red;
      break;

    case violet_to_red:
      currentB -= 1;
      analogWrite(BLUE_LED_OUT, currentB);
      if (currentR == minBrightness)
        fade_state = red_to_yellow;
      break;

    case red_to_yellow:
      currentG += 1;
      analogWrite(GREEN_LED_OUT, currentG);
      if (currentG == maxBrightness)
        fade_state = yellow_to_green;
      break;

    case yellow_to_green:
      currentR -= 1;
      analogWrite(RED_LED_OUT, currentR);
      if (currentR == minBrightness)
        fade_state = green_to_teal;
      break;

    case green_to_teal:
      currentB += 1;
      analogWrite(BLUE_LED_OUT, currentB);
      if (currentB == maxBrightness)
        fade_state = teal_to_blue;
      break;

    case teal_to_blue:
      currentG -= 1;
      analogWrite(GREEN_LED_OUT, currentG);
      if (currentG == minBrightness)
        fade_state = blue_to_violet;
      break;
  }
}

void turnOn( ) 
{ 
  setColorToLevel( "all", maxBrightness );
}

void turnOff( ) 
{
  setColorToLevel( "all", offBrightness );
}

void setColorToLevel( String color, int level ) 
{ 
  int redPin = RED_LED_OUT;
  int greenPin = GREEN_LED_OUT;
  int bluePin = BLUE_LED_OUT;
  
  if( color == "red" )
  {
    analogWrite( redPin, level );
    currentR = level;
  }
  if( color == "green" )
  {
    analogWrite( greenPin, level );
    currentG = level;
  }
  if( color == "blue" )
  {
    analogWrite( bluePin, level );
    currentB = level;
  }
  if( color == "all" )
  {
    analogWrite( redPin, level );
    analogWrite( greenPin, level );
    analogWrite( bluePin, level );
    currentR = level;
    currentG = level;
    currentB = level;
  }

  currentRFirst = currentR;
  currentGFirst = currentG;
  currentBFirst = currentB;
}

void loop() {
  int r, g, b;

  digitalWrite(MOTION_DETECTED_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(MOTION_DETECTED_LED, LOW);    // turn the LED off by making the voltage LOW
  delay(200);                       // wait for a second

  // fade from blue to violet
  for (r = 0; r < maxBrightness; r++) { 
    analogWrite(RED_LED_OUT, r);
    delay(FADESPEED);
  } 
  // fade from violet to red
  for (b = maxBrightness; b > 0; b--) { 
    analogWrite(BLUE_LED_OUT, b);
    delay(FADESPEED);
  } 
  // fade from red to yellow
  for (g = 0; g < maxBrightness; g++) { 
    analogWrite(GREEN_LED_OUT, g);
    delay(FADESPEED);
  } 
  // fade from yellow to green
  for (r = maxBrightness; r > 0; r--) { 
    analogWrite(RED_LED_OUT, r);
    delay(FADESPEED);
  } 
  // fade from green to teal
  for (b = 0; b < maxBrightness; b++) { 
    analogWrite(BLUE_LED_OUT, b);
    delay(FADESPEED);
  } 
  // fade from teal to blue
  for (g = maxBrightness; g > 0; g--) { 
    analogWrite(GREEN_LED_OUT, g);
    delay(FADESPEED);
  } 
}
void loop2() 
{
//  currentR += 10;
//  if (currentR >= maxBrightness)
//    currentR = 0;
//  analogWrite( RED_LED_OUT, currentR );
//  currentB += 20;
//  if (currentB >= maxBrightness)
//    currentB = 0;
//  analogWrite( BLUE_LED_OUT, currentB );
//  currentG += 30;
//  if (currentG >= maxBrightness)
//    currentG = 0;
//  analogWrite( GREEN_LED_OUT, currentG );
  Serial.println("Red On");
  setColorToLevel( "red", maxBrightness );
  delay( 1000 );
  Serial.println("Red Off");
  setColorToLevel( "red", offBrightness );
  delay( 1000 );
  Serial.println("Blue On");
  setColorToLevel( "blue", maxBrightness );
  delay( 1000 );
  Serial.println("Blue Off");
  setColorToLevel( "blue", offBrightness );
  delay( 1000 );
  Serial.println("Green On");
  setColorToLevel( "green", maxBrightness );
  delay( 1000 );
  Serial.println("Green Off");
  setColorToLevel( "green", offBrightness );
  delay( 1000 );
  Serial.println("Turn off");
  turnOff();
  delay(1000);

}
