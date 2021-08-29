
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <DHT.h>

#include "/home/suzi/src/sketches/defs/sierra_wifi_defs.h"
#define version_str "v0.1.1-20210829"

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
const unsigned long Sample_period_ms = 1000;

#define DHTTYPE DHT11
#define DHTPIN  2

/*
**  I/O variables...
*/
// GPIO assignments
const int out_HB_LED = LED_BUILTIN;
//const int out_HB_LED = D8;

DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
float humidity, temp_f; 

void setup() {
  Serial.begin( 57600 );

  dht.begin();  

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

  server.on("/update", []() {
    readTemp();
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
    readTemp();
  }
  
  server.handleClient();
  MDNS.update();
}

void readTemp()
{
  humidity = dht.readHumidity();          // Read humidity (percent)
  temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temp_f)) {
    humidity =-99;
    temp_f =-99;
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
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Default Server</h1>\n";
  ptr +="<h6>version: ";
  ptr +=version_str;
  ptr +="</h6>\n";
  
  if( state) {
  } else {
    ptr += "<div id=\"container\"><a class=\"button button-on\" href=\"/update\">UPDATE</a></div>\n";
  }

  ptr +="<tr><td>Temp: </td><td>"+String(temp_f)+"</td></tr>\n";
  ptr +="<tr><td>Humidity: </td><td>"+String(humidity)+"</td></tr>\n";
  ptr +="<tr><td>Last update at: </td><td>"+String(lastReadMillis)+"</td></tr>\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
