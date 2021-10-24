
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#include <Wire.h>
#include "Adafruit_VL6180X.h"

#include "/home/suzi/src/sketches/defs/sierra_wifi_defs.h"
#define version_str "v0.1.2-20210829"

/*
**  Network variables...
*/
IPAddress ip(192, 168, 129, POOL_CHECK_IP_LAST_FIELD);  // make sure IP is *outside* of DHCP pool range
IPAddress gateway(192, 168, 129, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 129, 254);
const char* ssid     = SSID;
const char* password = WIFI_PW;
int server_port = 80;
String DNS_name = DEFAULT_HOSTNAME;

// Set web server port number
ESP8266WebServer server(server_port);

/*
**  global variables...
*/
uint8_t state = true;
unsigned long prevMillis;
unsigned long lastReadMillis;
const unsigned long HB_period_ms = 1000;
const unsigned long Sample_period_ms = 500;

/*
**  I/O variables...
*/
// GPIO assignments
const int out_HB_LED = LED_BUILTIN;
//const int out_HB_LED = D8;

Adafruit_VL6180X vl = Adafruit_VL6180X();
float dist_mm;
String lastStatus;
bool sensorFound;
long retryCnt;
bool autoUpdate;

void setup() {
  Serial.begin( 57600 );

  // for ESP01s, we need to setup SDA to GPIO2
  //  Note: Connect SCL to GPIO1, and SDA to GPIO2
  Wire.begin(2,0);

  autoUpdate = true;
  
  retryCnt = 0;
  Serial.println("Can we find sensor?");
  if (! vl.begin()) {
    Serial.println("Failed to find sensor");
    lastStatus = "Failed to find sensor";
    sensorFound = false;
  } else {
    sensorFound = true;
    Serial.println("Sensor found!");
  }

  Serial.println("Configuring GPIO...");
  // Initialize the GPIO variables as outputs
  pinMode(out_HB_LED, OUTPUT);

  // blink the heartbeat LED a few times to indicate we're starting up wifi
  for (int i=0; i<3; ++i) {
    digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));
    delay(100);
  }

  // initialize wifi and connect...
  wifi_init();
  // uncomment to make never sleep:
  //WiFi.setSleepMode(WIFI_NONE_SLEEP);

  // setup OTA stuff...
  ArduinoOTA.setHostname(DEFAULT_HOSTNAME);
  ArduinoOTA.setPassword(DEFAULT_OTA_PW);

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

  server.on("/autoOn", []() {
    autoUpdate = true;
    String html_out = CreateHTML();
    server.send(200, "text/html", html_out);
  });

  server.on("/autoOff", []() {
    autoUpdate = false;
    String html_out = CreateHTML();
    server.send(200, "text/html", html_out);
  });

  server.on("/update", []() {
    if (sensorFound) {
      readDist();
    } else {
      retryCnt += 1;
      Serial.println("Retrying to find sensor...");
      if (! vl.begin()) {
        Serial.println("Failed to find sensor");
        lastStatus = "Failed to find sensor during retry";
        sensorFound = false;
      } else {
        sensorFound = true;
        Serial.println("Sensor found!");
      }
    }

    String html_out = CreateHTML();
    server.send(200, "text/html", html_out);
  });

   // Startup server
  server.begin();
  Serial.println("HTTP server started");

  // blink the heartbeat LED a few times to indicate we're ready
  for (int i=0; i<5; ++i) {
    digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));
    delay(100);
  }

  String tmp;
  tmp = "LED pin is: " + String(out_HB_LED);
  Serial.println(tmp);

  // get timestamp for heartbeat LED
  prevMillis = millis();
  lastReadMillis = millis();
}

void loop() {
  ArduinoOTA.handle();
  
  // First let's toggle the HB LED if 1000ms has elapsed
  unsigned long currentMillis = millis();  
  if (currentMillis - prevMillis >= HB_period_ms)  
  {
    digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));  //if so, change the state of the LED.  Uses a neat trick to change the state
    prevMillis = currentMillis;  
  }

  // update temp/humidity if it's been longer than sample period
  unsigned long currentMillis2 = millis();  
  if (currentMillis2 - lastReadMillis >= Sample_period_ms)
  {
    if (sensorFound) {
      readDist();
    } else {
      retryCnt += 1;
      Serial.println("Retrying to find sensor...");
      if (! vl.begin()) {
        Serial.println("Failed to find sensor");
        lastStatus = "Failed to find sensor during retry";
        sensorFound = false;
      } else {
        sensorFound = true;
        Serial.println("Sensor found!");
      }
    }
  }
  
  server.handleClient();
  MDNS.update();
}

void readDist()
{
  float lux = vl.readLux(VL6180X_ALS_GAIN_5);

  //Serial.print("Lux: "); Serial.println(lux);

  uint8_t range = vl.readRange();
  uint8_t status = vl.readRangeStatus();

  if (status == VL6180X_ERROR_NONE) {
    Serial.print("Range: "); Serial.println(range);
    lastStatus = "Good read";
    dist_mm = range;
    
    String graph = String(range);
    for (int i =0; i<range; i++)
      graph += "#";

    Serial.println(graph); 
  } else {
    dist_mm = -99;
  }

  // Some error occurred, print it out!

  if  ((status >= VL6180X_ERROR_SYSERR_1) && (status <= VL6180X_ERROR_SYSERR_5)) {
    Serial.println("System error");
    lastStatus = "System error";
  }
  else if (status == VL6180X_ERROR_ECEFAIL) {
    Serial.println("ECE failure");
    lastStatus = "ECE failure";
  }
  else if (status == VL6180X_ERROR_NOCONVERGE) {
    Serial.println("No convergence");
    lastStatus = "No convergence";
  }
  else if (status == VL6180X_ERROR_RANGEIGNORE) {
    Serial.println("Ignoring range");
    lastStatus = "Ignoring range";
  }
  else if (status == VL6180X_ERROR_SNR) {
    Serial.println("Signal/Noise error");
    lastStatus = "Signal/Noise error";
  }
  else if (status == VL6180X_ERROR_RAWUFLOW) {
    Serial.println("Raw reading underflow");
    lastStatus = "Raw reading underflow";
  }
  else if (status == VL6180X_ERROR_RAWOFLOW) {
    Serial.println("Raw reading overflow");
    lastStatus = "Raw reading overflow";
  }
  else if (status == VL6180X_ERROR_RANGEUFLOW) {
    Serial.println("Range reading underflow");
    lastStatus = "Range reading underflow";
  }
  else if (status == VL6180X_ERROR_RANGEOFLOW) {
    Serial.println("Range reading overflow");
    lastStatus = "Range reading overflow";
  }
  
  lastReadMillis = millis();
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
  ptr +="<title>Default Server</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: inline-block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 8px 20px;text-decoration: none;font-size: 25px;margin: 0px auto 25px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";

  if( autoUpdate) {
    ptr +="<script>";
    ptr +="function refresh(refreshPeriod) { setTimeout('location.reload(true)', refreshPeriod); }";
    ptr +="window.onload = refresh(500);";
    ptr +="</script>\n";
  }

  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Default Server</h1>\n";
  ptr +="<h6>version: ";
  ptr +=version_str;
  ptr +="</h6>\n";
  
  ptr += "<div id=\"container\"><a class=\"button button-on\" href=\"/update\">UPDATE</a></div>\n";

  if( autoUpdate) {    
     ptr += "<div id=\"container\"><a class=\"button button-on\" href=\"/autoOff\">Auto Off</a>\n";
  } else {
     ptr += "<div id=\"container\"><a class=\"button button-off\" href=\"/autoOn\">Auto On</a>\n";
  }

  ptr +="<table><caption>Debug Info</caption><tbody>\n";
  //ptr +="<thead><tr><th>Variable</th><th>Value</th></tr></thead>\n";
  ptr +="<tbody>";
  ptr +="<tr><td>Range: </td><td>"+String(dist_mm)+"</td></tr>\n";
  ptr +="<tr><td>Last update at: </td><td>"+String(lastReadMillis)+"</td></tr>\n";
  ptr +="<tr><td>sensorFound: </td><td>"+String(sensorFound)+"</td></tr>\n";
  ptr +="<tr><td>Last status: </td><td>"+lastStatus+"</td></tr>\n";
  ptr +="<tr><td>retryCnt: </td><td>"+String(retryCnt)+"</td></tr>\n";
  ptr +="</tbody></table>\n";






  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
