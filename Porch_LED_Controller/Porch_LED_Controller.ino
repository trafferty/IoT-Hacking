#include <Time.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "../../defs/sierra_wifi_defs.h"

// forward declarations...
void wifi_init();
time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

/*
**  Network variables...
*/
IPAddress ip(192, 168, 129, PORCH_IP_LAST_FIELD);  // make sure IP is *outside* of DHCP pool range
IPAddress gateway(192, 168, 129, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 129, 254);
const char* ssid     = SSID;
const char* password = WIFI_PW;

WiFiUDP Udp;
unsigned int UdpPort = 8888;  // local port to listen for UDP packets

// structs to hold start/end times for scheduling.
tmElements_t tmStart1;
tmElements_t tmStart2;
tmElements_t tmEnd1;
tmElements_t tmEnd2;
tmElements_t tmMotionAlertStart;
tmElements_t tmMotionAlertEnd;

// Pin mapping for the first string of lights. Pins D5 - D7
#define BLUE_LED_OUT         D8
#define RED_LED_OUT          D6
#define GREEN_LED_OUT        D5
#define MOTION_DETECTED_LED  D0
#define PIR_IN               D1

#define FADESPEED 3     // make this higher to slow down

typedef enum {
    blue_to_violet, violet_to_red, red_to_yellow, yellow_to_green, green_to_teal, teal_to_blue
} LED_Fade_States_t;

typedef enum {
    state_idle, state_running 
} Run_States_t;

const int maxBrightness = 1023;
//int maxBrightness = 500;
const int minBrightness = 0;

bool NTPTimeSet;

// Vars for holding LED current values and current fade state
int currentR;
int currentG;
int currentB;
LED_Fade_States_t fade_state;
Run_States_t      run_state;

void setup() 
{
  Serial.begin( 57600 );
  delay( 100 );

  wifi_init();

  Serial.println("Starting UDP");
  Udp.begin(UdpPort);
  Serial.print("UDP port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(5);
  NTPTimeSet = false;

  // Prepare the pins to fire
  pinMode( BLUE_LED_OUT, OUTPUT );
  pinMode( RED_LED_OUT, OUTPUT );
  pinMode( GREEN_LED_OUT, OUTPUT );
  pinMode( MOTION_DETECTED_LED, OUTPUT );
  pinMode( PIR_IN, INPUT );

  currentR = minBrightness;
  currentG = minBrightness;
  currentB = minBrightness;
  fade_state = blue_to_violet;
  run_state  = state_idle;

  allLEDsOff();
  
  // setup start/end time structs for scheduler
  tmStart1.Second = 0;
  tmStart1.Minute = 0;
  tmStart1.Hour   = 18; // 6:00pm
  tmEnd1.Second = 0;
  tmEnd1.Minute = 0;
  tmEnd1.Hour   = 23;   // 11:00pm
  tmStart2.Second = 0;
  tmStart2.Minute = 15;
  tmStart2.Hour   = 6;  // 6:15am
  tmEnd2.Second = 0;
  tmEnd2.Minute = 45;
  tmEnd2.Hour   = 8;    // 6:45am
  tmMotionAlertStart.Second = 1;
  tmMotionAlertStart.Minute = 0;
  tmMotionAlertStart.Hour   = 0;  // 12:00am
  tmMotionAlertEnd.Second = 0;
  tmMotionAlertEnd.Minute = 0;
  tmMotionAlertEnd.Hour   = 6;    // 6:00am

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

void allLEDsValue(int value)
{
  analogWrite(RED_LED_OUT, value);
  analogWrite(GREEN_LED_OUT, value);
  analogWrite(BLUE_LED_OUT, value);
}

void allLEDsOff()
{
  digitalWrite(RED_LED_OUT, LOW);
  digitalWrite(GREEN_LED_OUT, LOW);
  digitalWrite(BLUE_LED_OUT, LOW);
}

void allLEDsOn()
{
  allLEDsValue(maxBrightness);
}

void loop() 
{
  // make sure the rest of the scheduling time struct fields are set with proper year, month, day
  tmStart1.Year = tmEnd1.Year = tmStart2.Year = tmEnd2.Year = \
      tmMotionAlertStart.Year = tmMotionAlertEnd.Year = year() - 1970;
  tmStart1.Month = tmEnd1.Month = tmStart2.Month = tmEnd2.Month = \
      tmMotionAlertStart.Month = tmMotionAlertEnd.Month = month();
  tmStart1.Day = tmEnd1.Day = tmStart2.Day = tmEnd2.Day = \
       tmMotionAlertStart.Day = tmMotionAlertEnd.Day= day();

  time_t start1_time = makeTime(tmStart1);
  time_t end1_time = makeTime(tmEnd1);
  time_t start2_time = makeTime(tmStart2);
  time_t end2_time = makeTime(tmEnd2);
  time_t motionAlertStart_time = makeTime(tmMotionAlertStart);
  time_t motionAlertEnd_time = makeTime(tmMotionAlertEnd);
  time_t now_time = now();

  if (timeStatus() != timeNotSet)
  {
    if (!NTPTimeSet)
    {
      setSyncInterval(3600);
      NTPTimeSet = true;
    }

    if ( ((now_time >= start1_time) && (now_time <= end1_time)) ||
         ((now_time >= start2_time) && (now_time <= end2_time)) ) 
    {
      if (run_state == state_idle)
      {
        Serial.print("Starting LED Fade program at ");
        digitalClockDisplay();

        // setup starting config for fade state machine
        currentR = minBrightness;
        currentG = minBrightness;
        currentB = maxBrightness;
        analogWrite(BLUE_LED_OUT, currentB);
        fade_state = blue_to_violet;

        run_state = state_running; 
      }

      fadeLEDs();
    } 
    else 
    {
      if (run_state == state_running)
      {
          Serial.print("Ending LED Fade program at ");
          digitalClockDisplay();
          allLEDsOff();
          run_state = state_idle;
      }
    }
  }
  else
  {
    if (NTPTimeSet)
    {
      setSyncInterval(5);
      NTPTimeSet = false;
    }

    Serial.println("Time not yet set by NTP");
    allLEDsValue(100);
    delay(1000);
    allLEDsValue(minBrightness);
  }

  if (digitalRead(PIR_IN) == HIGH)
  {
    Serial.println("Motion detected!");
    digitalWrite(MOTION_DETECTED_LED, HIGH);

    if (timeStatus() != timeNotSet)
    {
      if((run_state == state_idle) && ((now_time >= motionAlertStart_time) && (now_time <= motionAlertEnd_time)))
        allLEDsOn();
    }
  }
  else
  {
    digitalWrite(MOTION_DETECTED_LED, LOW);

    if (timeStatus() != timeNotSet)
    {
      if((run_state == state_idle) && ((now_time >= motionAlertStart_time) && (now_time <= motionAlertEnd_time)))
        allLEDsOff();
    }
  }

  delay(FADESPEED);
}

/* ----- Util functions ----- */
void wifi_init()
{
  Serial.print("Setting up network with static IP: ");
  //Serial.println(ip);
  WiFi.config(ip, gateway, subnet, DNS);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(200);
  }
  Serial.println();
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Fail connecting");
    delay(5000);
    ESP.restart();
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// NTP Servers:
//static const char ntpServerName[] = "us.pool.ntp.org";
static const char ntpServerName[] = "time.nist.gov";
const int timeZone = -5;     // Central Standard Time

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  delay(20);

  uint8_t attempt = 0;
  uint8_t maxAttempts = 3;
  while (attempt < maxAttempts) {
    attempt++;
    uint32_t beginWait = millis();
    while (millis() - beginWait < 3000) {
      int size = Udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        Serial.println(attempt);
        Serial.println("Receive NTP Response");
        Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
        unsigned long secsSince1900;
        // convert four bytes starting at location 40 to a long integer
        secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      }
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
