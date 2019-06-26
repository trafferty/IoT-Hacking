

// Define Pins
#define BLUE 3
#define GREEN 5
#define RED 6

void setup()
{
pinMode(RED, OUTPUT);
pinMode(GREEN, OUTPUT);
pinMode(BLUE, OUTPUT);
digitalWrite(RED, HIGH);
digitalWrite(GREEN, LOW);
digitalWrite(BLUE, LOW);
}

// define variables
int redValue;
int greenValue;
int blueValue;

struct COLOR {
  int red;
  int green;
  int blue;
};
// main loop
void loop()
{
  #define delayTime 10 // fading time between colors
  
  COLOR purple;
  purple.red = 51;
  purple.green = 102;
  purple.blue = 0;
  
  analogWrite(RED, purple.red);
  analogWrite(GREEN, purple.green);
  analogWrite(BLUE, purple.blue);

}
