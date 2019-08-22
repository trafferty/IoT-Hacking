/*********
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "/home/suzi/src/sketches/defs/sierra_wifi_defs.h"

/*
**  Network variables...
*/
// NETWORK: Static IP and WIFI details...
IPAddress ip(192, 168, 129, 200);  // make sure IP is *outside* of DHCP pool range
IPAddress gateway(192, 168, 129, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 129, 254);
int server_port = 80;

// Set web server port number
ESP8266WebServer server(server_port);
// Variable to store the HTTP request
String header;

const int led = 4;

long randNumber;

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

void handleRoot() {
  digitalWrite(led, 1);
  blinkLED(LED_BUILTIN, 250, 2);
  blinkLED(led, 250, 2);

  String message = "hey yo! from esp8266!  ";
  message += String(randNumber);
  message += ", LED_BUILTIN: ";
  message += String(LED_BUILTIN);
  
  server.send(200, "text/plain", message);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
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
  digitalWrite(led, 0);
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

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(led, 0);
  Serial.begin(115200);
  wifi_init();

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  randomSeed(analogRead(0));
  
  blinkLED(LED_BUILTIN, 200, 3);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {

  randNumber = random(30000);
  //Serial.println(randNumber);
  
  server.handleClient();
  MDNS.update();
}
