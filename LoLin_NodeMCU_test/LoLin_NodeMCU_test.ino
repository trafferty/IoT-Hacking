#include "ESP8266WiFi.h"

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

// GPIO assignments
const int out_HB_LED = 2;
unsigned long prevMillis;
const unsigned long HB_period_ms = 1000;

void blinkLED(int pin, int delay_ms, int cnt)
{
  for (int i=0; i<cnt; ++i)
  {
    digitalWrite(pin, LOW); 
    delay(delay_ms);                      
    digitalWrite(pin, HIGH);
    if (i+1<cnt)
      delay(delay_ms);                      
  }
}

void setup() {
  Serial.begin(115200);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  pinMode(out_HB_LED, OUTPUT);
  // blink the heartbeat LED a few times to indicate we're starting up
  for (int i=0; i<7; ++i) {
    digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));
    delay(100);
  }

  delay(2000);
  Serial.println("Setup done");
}

void loop() {
  blinkLED(out_HB_LED, 100, 3);

  Serial.println("scan start");
  
  int n = WiFi.scanNetworks();// WiFi.scanNetworks will return the number of networks found
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");

  // Wait a bit before scanning again
  delay(5000);
}
