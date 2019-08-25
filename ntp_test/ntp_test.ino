/*
 * TimeNTP_ESP8266WiFi.ino
 * Example showing time sync to NTP time source
 *
 * This sketch uses the ESP8266WiFi library
 */

#include <Time.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


#include "/home/suzi/src/sketches/defs/sierra_wifi_defs.h"

/*
**  Network variables...
*/
IPAddress ip(192, 168, 129, 222);  // make sure IP is *outside* of DHCP pool range
IPAddress gateway(192, 168, 129, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 129, 254);
const char* ssid     = SSID;
const char* password = WIFI_PW;

// NTP Servers:
//static const char ntpServerName[] = "us.pool.ntp.org";
static const char ntpServerName[] = "time.nist.gov";
const int timeZone = -5;     // Central Standard Time

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

tmElements_t tmStart1;
tmElements_t tmStart2;
tmElements_t tmEnd1;
tmElements_t tmEnd2;

uint8_t NTPTimeSet;

void setup()
{
  Serial.begin(9600);
  delay(250);
  Serial.println("TimeNTP Example");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(5);
  NTPTimeSet = false;

  tmStart1.Second = 0;
  tmStart1.Minute = minute() + 1;
  tmStart1.Hour = hour();
  tmEnd1.Second = 0;
  tmEnd1.Minute = tmStart1.Minute + 1;
  tmEnd1.Hour = tmStart1.Hour;
}

time_t prevDisplay = 0; // when the digital clock was displayed

void loop_orig()
{
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
}

void loop()
{
  digitalClockDisplay();

  if (timeStatus() == timeNotSet)
  {
    if (!NTPTimeSet)
    {
      setSyncInterval(300);
      NTPTimeSet = true;
    }

    tmStart1.Year = tmEnd1.Year = year() - 1970;
    tmStart1.Month = tmEnd1.Month = month();
    tmStart1.Day = tmEnd1.Day = day();

    time_t start_time = makeTime(tmStart1);
    time_t end_time = makeTime(tmEnd1);
    time_t now_time = now();
    //  Serial.print("alarm_time: ");
    //  Serial.println(alarm_time);
    //  Serial.print("now_time: ");
    //  Serial.println(now_time);

    if ((now_time >= start_time) && (now_time <= end_time)) {
      Serial.println(": Alarm!!");
    }
  }
  else
  {
    NTPTimeSet = false;
    Serial.println(": Time not yet set by NTP");
  }

  delay(1000);
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
