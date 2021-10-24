#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int servo = 7;
int home_sw = 6;      // home/limit switch
int toggle_sw = 5;    // toggle switch
int out_HB_LED = 4;

int rate = 5;

bool doWait;
int waitTime_ms = 1000;

int val;    // variable to read the value from the analog pin
int pos = 0;

void setup() {
    //Serial Port begin
  Serial.begin (57000);

  myservo.attach(servo);  // attaches the servo on pin 9 to the servo object

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));

  pinMode(toggle_sw, INPUT_PULLUP);
  pinMode(home_sw, INPUT_PULLUP);

  doWait = true;
  pos = 85;
  myservo.write(pos);
}

void loop() {

  int toggleVal = digitalRead(toggle_sw);
  int homeVal = digitalRead(home_sw);

  if (toggleVal == 0)
  {
    pos += rate;
    if (pos >= 85)
      pos = 85;

    doWait = true;
  }
  else if ((toggleVal == 1))
  {
    if (doWait)
    {
      waitTime_ms = random(100, 1500);
      Serial.print("--------waiting: ");
      Serial.print(waitTime_ms);
      Serial.println(" ---------");
      delay(waitTime_ms);
      doWait = false;
    }
    pos -= rate;
    if (pos <= 0)
      pos = 0;
  }
 
  myservo.write(pos); 
  delay(2);                           // waits for the servo to get there
  pos = myservo.read();
  
  Serial.print(pos);
  Serial.print("\ttoggle: ");
  Serial.print(toggleVal);
  Serial.print("\thome: ");
  Serial.println(homeVal);

}
