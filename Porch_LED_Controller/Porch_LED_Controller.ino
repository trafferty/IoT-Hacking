/*
  This is the more advanced Porch Light controller.  It has webserver,
  NTP time, and follows a preset schedule to run certain LED routines.
*/

#include <Time.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#include "/home/suzi/src/sketches/defs/sierra_wifi_defs.h"
#define version_str "v0.9.6-20200830"

// forward declarations...
void wifi_init();
time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

/*
**  Network variables...
*/
IPAddress ip(192, 168, 129, PORCH_LIGHT_IP_LAST_FIELD);  // make sure IP is *outside* of DHCP pool range
IPAddress gateway(192, 168, 129, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 129, 254);
const char* ssid     = SSID;
const char* password = WIFI_PW;
int server_port = 80;
String DNS_name = GARAGE_LIGHT_HOSTNAME;

// Set web server port number
ESP8266WebServer server(server_port);

WiFiUDP Udp;
unsigned int UdpPort = 8888;  // local port to listen for UDP packets

typedef enum {
    ON, OFF
} LIGHT_STATE_t;

typedef enum {
    blue_to_violet, violet_to_red, red_to_yellow, yellow_to_green, green_to_teal, teal_to_blue
} LED_Fade_States_t;

typedef enum {
    blink_blue, blink_red, blink_yellow, blink_green, blink_teal, blink_violet
} Blink_States_t;

typedef enum {
    state_idle, state_running 
} Run_States_t;

typedef enum {
    program_fade, program_random, program_blink 
} LED_program_t;

// Pin mapping for the first string of lights. Pins D5 - D7
#define BLUE_LED_OUT         D8
#define RED_LED_OUT          D6
#define GREEN_LED_OUT        D5
#define MOTION_DETECTED_LED  D0
#define PIR_IN               D1

#define FADESPEED 3     // make this higher to slow down

/*
**  global variables...
*/
uint8_t light_state = ON;
uint8_t fade_speed = FADESPEED;

// structs to hold start/end times for scheduling.
tmElements_t tmStart1;
tmElements_t tmStart2;
tmElements_t tmEnd1;
tmElements_t tmEnd2;
tmElements_t tmMotionAlertStart;
tmElements_t tmMotionAlertEnd;
time_t start1_time;
time_t end1_time;
time_t start2_time;
time_t end2_time;
time_t motionAlertStart_time;
time_t motionAlertEnd_time;
time_t now_time;

const int maxBrightness = 1023;
//int maxBrightness = 500;
const int minBrightness = 0;

bool NTPTimeSet;

// Vars for holding LED current values and current fade state
int currentR;
int currentG;
int currentB;
LED_Fade_States_t fade_state;
Blink_States_t    blink_state;
Run_States_t      run_state;
LED_program_t     led_program;

void setup() 
{
  Serial.begin( 57600 );
  delay( 100 );

  wifi_init();
  // uncomment to make never sleep:
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  // setup OTA stuff...
  ArduinoOTA.setHostname(PORCH_LIGHT_HOSTNAME);
  ArduinoOTA.setPassword(PORCH_LIGHT_OTA_PW);

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");

  // setup DNS handler
  if (MDNS.begin(DNS_name)) {
    Serial.println("mDNS responder started. Connect at http:\\\\"+DNS_name+".local");
  } else {
    Serial.println("mDNS responder NOT started.");    
  }

  // setup all the web server handlers
  server.onNotFound(handleNotFound);

  server.on("/", []() {
    String html_out = CreateHTML();
    server.send(200, "text/html", html_out);
  });
  server.on("/light_on", []() {
    toggle_light(ON);
    server.send(200, "text/html", CreateHTML());
  });
  server.on("/light_off", []() {
    toggle_light(OFF);
    server.send(200, "text/html", CreateHTML());
  });
  server.on("/fade_ultra_slow", []() {
    fade_speed = 10;
    server.send(200, "text/html", CreateHTML());
  });
  server.on("/fade_slow", []() {
    fade_speed = 5;
    server.send(200, "text/html", CreateHTML());
  });
  server.on("/fade_medium", []() {
    fade_speed = 3;
    server.send(200, "text/html", CreateHTML());
  });
  server.on("/fade_fast", []() {
    fade_speed = 0;
    server.send(200, "text/html", CreateHTML());
  });

   // Startup server
  server.begin();
  Serial.println("HTTP server started");

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
  blink_state = blink_blue;
  run_state  = state_idle;

  led_program = program_fade;
  allLEDsOff();
  
  // setup start/end time structs for scheduler
  tmStart1.Second = 0;
  tmStart1.Minute = 30;
  tmStart1.Hour   = 19; // 7:00pm
  tmEnd1.Second = 0;
  tmEnd1.Minute = 0;
  tmEnd1.Hour   = 23;   // 11:00pm
  tmStart2.Second = 0;
  tmStart2.Minute = 15;
  tmStart2.Hour   = 6;  // 6:15am
  tmEnd2.Second = 0;
  tmEnd2.Minute = 45;
  tmEnd2.Hour   = 6;    // 6:45am
  tmMotionAlertStart.Second = 0;
  tmMotionAlertStart.Minute = 0;
  tmMotionAlertStart.Hour   = 0;  // 12:00am
  tmMotionAlertEnd.Second = 0;
  tmMotionAlertEnd.Minute = 0;
  tmMotionAlertEnd.Hour   = 6;    // 6:00am

  digitalWrite(MOTION_DETECTED_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(MOTION_DETECTED_LED, LOW);    // turn the LED off by making the voltage LOW
}

void loop() 
{
  ArduinoOTA.handle();

  // make sure the rest of the scheduling time struct fields are set with proper year, month, day
  tmStart1.Year = tmEnd1.Year = tmStart2.Year = tmEnd2.Year = \
      tmMotionAlertStart.Year = tmMotionAlertEnd.Year = year() - 1970;
  tmStart1.Month = tmEnd1.Month = tmStart2.Month = tmEnd2.Month = \
      tmMotionAlertStart.Month = tmMotionAlertEnd.Month = month();
  tmStart1.Day = tmEnd1.Day = tmStart2.Day = tmEnd2.Day = \
       tmMotionAlertStart.Day = tmMotionAlertEnd.Day= day();

  start1_time = makeTime(tmStart1);
  end1_time = makeTime(tmEnd1);
  start2_time = makeTime(tmStart2);
  end2_time = makeTime(tmEnd2);
  motionAlertStart_time = makeTime(tmMotionAlertStart);
  motionAlertEnd_time = makeTime(tmMotionAlertEnd);
  now_time = now();

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
        Serial.printf("Starting LED Fade program at %s\n", buildDateTimeStr(now_time).c_str());

        // setup starting config for fade state machine
        currentR = minBrightness;
        currentG = minBrightness;
        currentB = maxBrightness;
        analogWrite(BLUE_LED_OUT, currentB);
        fade_state = blue_to_violet;

        run_state = state_running; 
      }
    } 
    else 
    {
      if (run_state == state_running)
      {
          Serial.printf("Ending LED Fade program at %s\n", buildDateTimeStr(now_time).c_str());
          toggle_light(OFF);

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

  if (run_state == state_running)
  {
    switch (led_program)
    {
      case program_fade:
        fadeLEDs();
        break;

      case program_random:
        fadeLEDs();
        break;

      case program_blink:
        fadeLEDs();
        break;

      default:
        fadeLEDs();
    }
  }


#if 0
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
#endif

  delay(fade_speed);
  
  server.handleClient();
  MDNS.update();  
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

void blinkLEDs()
{

   // blink_blue, blink_red, blink_yellow, blink_green, blink_teal, blink_violet


  switch (blink_state)
  {
    case blink_blue:
      analogWrite(RED_LED_OUT, minBrightness);
      analogWrite(GREEN_LED_OUT, minBrightness);
      analogWrite(BLUE_LED_OUT, maxBrightness);
      blink_state = blink_red;
      Serial.println("Changing state to blink_red");
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

void toggle_light(LIGHT_STATE_t state)
{
  light_state = state;

  if (light_state == OFF)
  {
    allLEDsOff();
    run_state = state_idle;
  }
  else
  {
    run_state = state_running;
  } 
}
/* ----- Util functions ----- */
void wifi_init()
{
  Serial.print("Setting up network with static IP.");
  WiFi.config(ip, gateway, subnet, DNS);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Connect to Wi-Fi network with SSID and password
  Serial.printf("Connecting to %s", ssid);
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
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void handleNotFound() {
  String message = "404!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  Serial.println(message);

  // now redirect
  server.sendHeader("Location", "/",true);   //Redirect to our html web page  
  server.send(302, "text/plane","");
}

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

String CreateHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<link href=\"data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAA";
  ptr +="EAAAAAAAAAAAAAAA/4QAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEA";
  ptr +="ERABAAEAEBAQABAQEBABAAEAEQARABAQABAQABAAAQARAAEQARAAAAAAAAAAAAAAAAAAAAAAEREAEQAQAAAQAAEAEBAAABAAAAAQEAAAERAAEQ";
  ptr +="AREAAQAAEAABABABAAAQAQEAEAEREAEQAREAAAAAAAAAAAD//wAA2N0AAKuqAADdmQAArrsAANnMAAD//wAA//8AAIZvAAC9rwAAv68AAI5jAAC97QAAv";
  ptr +="a0AAIZjAAD//wAA\" rel=\"icon\" type=\"image/x-icon\" />\n";
  ptr +="<title>Porch Light Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: inline-block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 8px 20px;text-decoration: none;font-size: 25px;margin: 0px auto 25px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="table, th, td {border: 1px solid black;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Porch Light</h1>\n";
  ptr +="<h6>version: ";
  ptr +=version_str;
  ptr +="</h6>\n";
  ptr +="<h6>Current Time: ";
  ptr +=buildDateTimeStr(now());
  ptr +="</h6>\n";
  
  if( light_state == ON) {
    ptr +="<p>Light Status: ON</p><a class=\"button button-off\" href=\"/light_off\">OFF</a>\n";
  } else {
    ptr +="<p>Light Status: OFF</p><a class=\"button button-off\" href=\"/light_on\">ON</a>\n";
  }

  ptr +="<p>Fade Speed:</p><a class=\"button button-off\" href=\"/fade_ultra_slow\">Ultra Slow</a>\n";
  ptr +="<a class=\"button button-off\" href=\"/fade_slow\">Slow</a>\n";
  ptr +="<a class=\"button button-off\" href=\"/fade_medium\">Medium</a>\n";
  ptr +="<a class=\"button button-off\" href=\"/fade_fast\">Fast</a>\n";

  String time_status = (timeStatus()!=timeNotSet?"True":"False");
  String NTP_time_set = (NTPTimeSet?"True":"False");
  String run_state_ = (run_state==state_running?"Running":"Idle");
  String light_state_ = (light_state==ON?"On":"Off");
  
  ptr +="<table><caption>Debug Info</caption><tbody>\n";
  //ptr +="<thead><tr><th>Variable</th><th>Value</th></tr></thead>\n";
  ptr +="<tbody>";
  ptr +="<tr><td>On Range 1</td><td>"+buildTimeStr(makeTime(tmStart1))+"</td><td>"+buildTimeStr(makeTime(tmEnd1))+"</td></tr>\n";
  ptr +="<tr><td>On Range 1</td><td>"+String(start1_time)+"</td><td>"+String(end1_time)+"</td></tr>\n";
  ptr +="<tr><td>On Range 2</td><td>"+buildTimeStr(makeTime(tmStart2))+"</td><td>"+buildTimeStr(makeTime(tmEnd2))+"</td></tr>\n";
  ptr +="<tr><td>On Range 2</td><td>"+String(start2_time)+"</td><td>"+String(end2_time)+"</td></tr>\n";
  ptr +="<tr><td>Motion Range</td><td>"+buildTimeStr(makeTime(tmMotionAlertStart))+"</td><td>"+buildTimeStr(makeTime(tmMotionAlertEnd))+"</td></tr>\n";
  ptr +="<tr><td>Motion Range</td><td>"+String(motionAlertStart_time)+"</td><td>"+String(motionAlertEnd_time)+"</td></tr>\n";
  ptr +="<tr><td>Now</td><td>"+buildTimeStr(now_time)+"</td><td>"+String(now_time)+"</td></tr>\n";
  ptr +="<tr><td>Time set?</td><td>";
  ptr +=time_status+"</td></tr>\n";
  ptr +="<tr><td>NTP Time Set?</td><td>";
  ptr +=NTP_time_set+"</td></tr>\n";
  ptr +="<tr><td>Run State:</td><td>";
  ptr +=run_state_+"</td></tr>\n";
  ptr +="<tr><td>Light State:</td><td>";
  ptr +=light_state_+"</td></tr>\n";
  ptr +="<tr><td>LED Program:</td><td>";
  ptr +=String(led_program)+"</td></tr>\n";
  ptr +="</tbody></table>\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.println(buildDateTimeStr(now()));
}

String buildDateTimeStr(time_t t)
{
  String timeStr = formatNumber(hour(t));
  timeStr += ":" + formatNumber(minute(t));
  timeStr += ":" + formatNumber(second(t));
  timeStr += " " + formatNumber(month(t));
  timeStr += "_" + formatNumber(day(t));
  timeStr += "_" + formatNumber(year(t));
  return timeStr;
}
String buildTimeStr(time_t t)
{
  String timeStr = formatNumber(hour(t));
  timeStr += ":" + formatNumber(minute(t));
  timeStr += ":" + formatNumber(second(t));
  return timeStr;
}
String formatNumber(int num)
{
  String formatted_num = "";
  if (num < 10)
    formatted_num = "0";
  formatted_num += String(num);
  return formatted_num;
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
