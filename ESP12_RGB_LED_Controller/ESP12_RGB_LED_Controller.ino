/*
  This is just a basic RGB LED controller.
  No Network, no WiFi.  Just starts up and fades
  through RGB colors.

  This was used on Sierra porch light when I had it hooked
  up to a timer.  It just came on at certain times and did
  the fade routine.  Simple.
*/

// Pin mapping for the first string of lights. Pins D5 - D7
#define BLUE_LED_OUT      D8
#define RED_LED_OUT       D6
#define GREEN_LED_OUT     D5
#define MOTION_DETECTED_LED  D0

#define FADESPEED 3     // make this higher to slow down

typedef enum {
    blue_to_violet, violet_to_red, red_to_yellow, yellow_to_green, green_to_teal, teal_to_blue
} LED_Fade_States_t;

const int maxBrightness = 1023;
//int maxBrightness = 500;
const int minBrightness = 0;

// Vars for holding LED current values and current fade state
int currentR;
int currentG;
int currentB;
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

  currentR = minBrightness;
  currentG = minBrightness;
  currentB = maxBrightness;
  analogWrite(BLUE_LED_OUT, currentB);
  fade_state = blue_to_violet;

  digitalWrite(MOTION_DETECTED_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(MOTION_DETECTED_LED, LOW);    // turn the LED off by making the voltage LOW

}

void fadeLEDs()
{
  switch (fade_state)
  {
    case blue_to_violet:
    currentR += 1;
    analogWrite(RED_LED_OUT, currentR);
    if (currentR == maxBrightness) {
        fade_state = violet_to_red;
        Serial.println("Changing state to violet_to_red");
    } 
    break;

    case violet_to_red:
    currentB -= 1;
    analogWrite(BLUE_LED_OUT, currentB);
    if (currentB == minBrightness) {
        fade_state = red_to_yellow;
        Serial.println("Changing state to red_to_yellow");
    } 
    break;

    case red_to_yellow:
    currentG += 1;
    analogWrite(GREEN_LED_OUT, currentG);
    if (currentG == maxBrightness) {
        fade_state = yellow_to_green;
        Serial.println("Changing state to yellow_to_green");
    } 
    break;

    case yellow_to_green:
    currentR -= 1;
    analogWrite(RED_LED_OUT, currentR);
    if (currentR == minBrightness) {
        fade_state = green_to_teal;
        Serial.println("Changing state to green_to_teal");
    } 
    break;

    case green_to_teal:
    currentB += 1;
    analogWrite(BLUE_LED_OUT, currentB);
    if (currentB == maxBrightness) {
        fade_state = teal_to_blue;
        Serial.println("Changing state to teal_to_blue");
    } 
    break;

    case teal_to_blue:
    currentG -= 1;
    analogWrite(GREEN_LED_OUT, currentG);
    if (currentG == minBrightness) {
        fade_state = blue_to_violet;
        Serial.println("Changing state to blue_to_violet");
    } 
    break;

    default:
    Serial.println("fadeLEDs: state not supported");
    fade_state = blue_to_violet;
  }

  // Serial.print(currentR);
  // Serial.print(", ");
  // Serial.print(currentG);
  // Serial.print(", ");
  // Serial.println(currentB);
}

void loop() {
  fadeLEDs();
  delay(FADESPEED);
}
