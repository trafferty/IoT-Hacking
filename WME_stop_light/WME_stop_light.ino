/*********
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

/*
**  Network variables...
*/
// NETWORK: Static IP and WIFI details...
IPAddress ip(192, 168, 129, 200);  // make sure IP is *outside* of DHCP pool range
IPAddress gateway(192, 168, 129, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 129, 254);
const char* ssid     = "Scalpel4";
const char* password = "71372352127135124570917";
int server_port = 80;

// Set web server port number
ESP8266WebServer server(server_port);
// Variable to store the HTTP request
String header;

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

const char RED = 0;
const char ORANGE = 1;
const char GREEN = 2;
const char BLUE = 3;
const char ALARM = 4;

uint8_t red_state;
uint8_t orange_state;
uint8_t blue_state;
uint8_t green_state;
uint8_t alarm_state;

unsigned long prevMillis;
const unsigned long HB_period_ms = 1000;

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}

void wifi_init()
{
  Serial.print("Setting up network with static IP: ");
  Serial.println(IpAddress2String(ip));
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

void handleNotFound() {
  String message = "File Not Found\n\n";
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
  server.send(404, "text/plain", message);
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

void handle_LED_setState(uint8_t led, uint8_t state)
{
  switch (led)
  {
    case RED:
      Serial.print("red light: ");
      Serial.println(state);
      red_state = state;
      digitalWrite(out_RedLight, state);
      break;

    case ORANGE:
      Serial.print("orange light: ");
      Serial.println(state);
      orange_state = state;
      digitalWrite(out_OrangeLight, state);
      break;

    case GREEN:
      Serial.print("green light: ");
      Serial.println(state);
      green_state = state;
      digitalWrite(out_GreenLight, state);
      break;

    case BLUE:
      Serial.print("blue light: ");
      Serial.println(state);
      blue_state = state;
      digitalWrite(out_BlueLight, state);
      break;

    case ALARM:
      Serial.print("alarm: ");
      Serial.println(state);
      alarm_state = state;
      digitalWrite(out_Alarm, state);
      break;

    default:
      Serial.println("handle_LED_setState: invalid LED number");

  }
}

void setup(void) {
  Serial.begin(115200);
  wifi_init();

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

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
  red_state = LOW;
  digitalWrite(out_OrangeLight, LOW);
  orange_state = LOW;
  digitalWrite(out_BlueLight, LOW);
  blue_state = LOW;
  digitalWrite(out_GreenLight, LOW);
  green_state = LOW;
  digitalWrite(out_Alarm, LOW);

  // blink the heartbeat LED a few times to indicate we're starting up
  for (int i=0; i<6; ++i) {
    digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));
    delay(100);
  }

  // setup all the server handlers
  server.onNotFound(handleNotFound);

  server.on("/", []() {
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });
  
  server.on("/red_on", []() {
    handle_LED_setState(RED, HIGH);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });
  server.on("/red_off", []() {
    handle_LED_setState(RED, LOW);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });

  server.on("/orange_on", []() {
    handle_LED_setState(ORANGE, HIGH);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });
  server.on("/orange_off", []() {
    handle_LED_setState(ORANGE, LOW);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });

  server.on("/green_on", []() {
    handle_LED_setState(GREEN, HIGH);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });
  server.on("/green_off", []() {
    handle_LED_setState(GREEN, LOW);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });

  server.on("/blue_on", []() {
    handle_LED_setState(BLUE, HIGH);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });
  server.on("/blue_off", []() {
    handle_LED_setState(BLUE, LOW);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });

  server.on("/alarm_on", []() {
    handle_LED_setState(ALARM, HIGH);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });
  server.on("/alarm_off", []() {
    handle_LED_setState(ALARM, LOW);
    server.send(200, "text/html", SendHTML(red_state,orange_state,green_state,blue_state,alarm_state));
  });

  prevMillis = millis();

   // Startup server
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {  
  // First let's toggle the HB LED if 1000ms has elapsed
  unsigned long currentMillis = millis();  
  if (currentMillis - prevMillis >= HB_period_ms)  
  {
    digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));  //if so, change the state of the LED.  Uses a neat trick to change the state
    prevMillis = currentMillis;  
  }

//  Serial.print("server.uri():");
//  Serial.println(server.uri());

  server.handleClient();
  MDNS.update();
}

String SendHTML(
  uint8_t red_state, 
  uint8_t orange_state,
  uint8_t green_state,
  uint8_t blue_state,
  uint8_t alarm_state
){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Stop Light Server</h1>\n";
  //ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  
   if( red_state)
    {ptr +="<p>Red Status: ON</p><a class=\"button button-off\" href=\"/red_off\">OFF</a>\n";}
  else
    {ptr +="<p>Red Status: OFF</p><a class=\"button button-on\" href=\"/red_on\">ON</a>\n";}

  if (orange_state)
    {ptr +="<p>Orange Status: ON</p><a class=\"button button-off\" href=\"/orange_off\">OFF</a>\n";}
  else
    {ptr +="<p>Orange Status: OFF</p><a class=\"button button-on\" href=\"/orange_on\">ON</a>\n";}

  if (green_state)
    {ptr +="<p>Green Status: ON</p><a class=\"button button-off\" href=\"/green_off\">OFF</a>\n";}
  else
    {ptr +="<p>Green Status: OFF</p><a class=\"button button-on\" href=\"/green_on\">ON</a>\n";}

  if (blue_state)
    {ptr +="<p>Blue Status: ON</p><a class=\"button button-off\" href=\"/blue_off\">OFF</a>\n";}
  else
    {ptr +="<p>Blue Status: OFF</p><a class=\"button button-on\" href=\"/blue_on\">ON</a>\n";}

  if (alarm_state)
    {ptr +="<p>Alarm Status: ON</p><a class=\"button button-off\" href=\"/alarm_off\">OFF</a>\n";}
  else
    {ptr +="<p>Alarm Status: OFF</p><a class=\"button button-on\" href=\"/alarm_on\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
