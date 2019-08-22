
/*
**  I/O variables...
*/
// GPIO assignments
const int out_HB_LED = LED_BUILTIN;
const int out_RedLight = 4;
const int out_OrangeLight = 5;
const int out_BlueLight = 12;
const int out_GreenLight = 13;
const int out_Alarm = 14;

unsigned long prevMillis;
const unsigned long HB_period_ms = 1000;

void blinkLED(int pin, int delay_ms, int cnt)
{
  for (int i=0; i<cnt; ++i)
  {
    digitalWrite(pin, !digitalRead(pin));
    delay(delay_ms);                      
    digitalWrite(pin, !digitalRead(pin));
    if (i+1<cnt)
      delay(delay_ms);                      
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Configuring GPIO and setting to low");
  // Initialize the GPIO variables as outputs

  pinMode(out_HB_LED, OUTPUT);
  pinMode(out_RedLight, OUTPUT);
  pinMode(out_OrangeLight, OUTPUT);
  pinMode(out_BlueLight, OUTPUT);
  pinMode(out_GreenLight, OUTPUT);
  pinMode(out_Alarm, OUTPUT);
  // Set outputs to LOW
  digitalWrite(out_RedLight, LOW);
  digitalWrite(out_OrangeLight, LOW);
  digitalWrite(out_BlueLight, LOW);
  digitalWrite(out_GreenLight, LOW);
  digitalWrite(out_Alarm, LOW);

  blinkLED(out_HB_LED, 200, 3);
  randomSeed(analogRead(0));
}

void loop() {
  // First let's toggle the HB LED if 1000ms has elapsed
  unsigned long currentMillis = millis();  
  if (currentMillis - prevMillis >= HB_period_ms)  
  {
    digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));  //if so, change the state of the LED.  Uses a neat trick to change the state
    prevMillis = currentMillis;  
  }

  digitalWrite(out_RedLight, (random(100) % 2));
  digitalWrite(out_OrangeLight, (random(100) % 2));
  digitalWrite(out_BlueLight, (random(100) % 2));
  digitalWrite(out_GreenLight, (random(100) % 2));
  digitalWrite(out_Alarm, (random(100) % 2));
  delay(50);
}
