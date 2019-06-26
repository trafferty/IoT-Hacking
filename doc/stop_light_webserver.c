/*********
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>

/*
**  Network variables...
*/
// NETWORK: Static IP and WIFI details...
IPAddress ip(192, 168, 129, 190);  // make sure IP is *outside* of DHCP pool range
IPAddress gateway(192, 168, 129, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 129, 249);
const char* ssid     = "Scalpel4";
const char* password = "YYYYYYY";
int server_port = 80;

// Set web server port number
WiFiServer server(server_port);
// Variable to store the HTTP request
String header;

/*
**  I/O variables...
*/
// GPIO assignments
const int out_HB_LED = 2;
const int out_RedLight = 4;
const int out_OrangeLight = 5;
const int out_BlueLight = 12;
const int out_GreenLight = 13;
const int out_Alarm = 14;

// Variables to store the current output state
String state_RedLight = "off";
String state_OrangeLight = "off";
String state_BlueLight = "off";
String state_GreenLight = "off";
String state_Alarm = "off";

unsigned long prevMillis;
const unsigned long HB_period_ms = 1000;

void wifi_init()
{
  Serial.print("Setting up network with static IP: ");
  Serial.println(ip);
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

void setup() 
{
  Serial.begin(115200);
  wifi_init();

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

  for (i=0; i<6; ++1) {
    digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));
    delay(200);
  }

  // Startup server
  server.begin();
  prevMillis = millis();
}

void loop() 
{
  // First let's toggle the HB LED if 1000ms has elapsed
  unsigned long currentMillis = millis();  
  if (currentMillis - prevMillis >= HB_period_ms)  
  {
    digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));  //if so, change the state of the LED.  Uses a neat trick to change the state
    prevMillis = currentMillis;  
  }

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client)                               // If a new client connects,
  {  
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client

    while (client.connected())             // loop while the client's connected
    {
      if (client.available())              // if there's bytes to read from the client,
      {
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;

        if (c == '\n')                    // if the byte is a newline character
        {  
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) 
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            // check for red
            if (header.indexOf("GET /red/on") >= 0) {
              Serial.println("red light on");
              state_RedLight = "on";
              digitalWrite(out_RedLight, HIGH);
            } else if (header.indexOf("GET /red/off") >= 0) {
              Serial.println("red light off");
              state_RedLight = "off";
              digitalWrite(out_RedLight, LOW);
            } 

            // check for orange
            else if (header.indexOf("GET /orange/on") >= 0) {
              Serial.println("orange light on");
              state_OrangeLight = "on";
              digitalWrite(out_OrangeLight, HIGH);
            } else if (header.indexOf("GET /orange/off") >= 0) {
              Serial.println("orange light off");
              state_OrangeLight = "off";
              digitalWrite(out_OrangeLight, LOW);
            } 

            // check for blue
            else if (header.indexOf("GET /blue/on") >= 0) {
              Serial.println("blue light on");
              state_BlueLight = "on";
              digitalWrite(out_BlueLight, HIGH);
            } else if (header.indexOf("GET /blue/off") >= 0) {
              Serial.println("blue light off");
              state_BlueLight = "off";
              digitalWrite(out_BlueLight, LOW);
            } 

            // check for green
            else if (header.indexOf("GET /green/on") >= 0) {
              Serial.println("green light on");
              state_GreenLight = "on";
              digitalWrite(out_GreenLight, HIGH);
            } else if (header.indexOf("GET /green/off") >= 0) {
              Serial.println("green light off");
              state_GreenLight = "off";
              digitalWrite(out_GreenLight, LOW);
            } 

            // check for alarm
            else if (header.indexOf("GET /alarm/on") >= 0) {
              Serial.println("alarm on");
              state_Alarm = "on";
              digitalWrite(out_Alarm, HIGH);
            } else if (header.indexOf("GET /alarm/off") >= 0) {
              Serial.println("alarm off");
              state_Alarm = "off";
              digitalWrite(out_Alarm, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Garage State Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for lights and alarm  
            client.println("<p>Red Light - State " + state_RedLight + "</p>");
            // If the state_RedLight is off, it displays the ON button       
            if (state_RedLight=="off") {
              client.println("<p><a href=\"/red/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/red/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            client.println("<p>Orange Light - State " + state_OrangeLight + "</p>");
            // If the state_OrangeLight is off, it displays the ON button       
            if (state_OrangeLight=="off") {
              client.println("<p><a href=\"/orange/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/orange/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for lights and alarm  
            client.println("<p>Blue Light - State " + state_BlueLight + "</p>");
            // If the state_BlueLight is off, it displays the ON button       
            if (state_BlueLight=="off") {
              client.println("<p><a href=\"/blue/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/blue/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for lights and alarm  
            client.println("<p>Green Light - State " + state_GreenLight + "</p>");
            // If the state_GreenLight is off, it displays the ON button       
            if (state_GreenLight=="off") {
              client.println("<p><a href=\"/green/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/green/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for lights and alarm  
            client.println("<p>Alarm - State " + state_Alarm + "</p>");
            // If the state_Alarm is off, it displays the ON button       
            if (state_Alarm=="off") {
              client.println("<p><a href=\"/alarm/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/alarm/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;




          }  else { // if you got a newline, then clear currentLine
            currentLine = "";
          }

        }  else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}