#include <Arduino.h>

#include <Time.h>
// #include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
// #include <ESP8266WebServer.h>
// #include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

/*
**  global variables...
*/

// GPIO assignments
const int in_GarageDoor = T8;
const int out_GarageLight = T8;
const int out_KitchenLight = T8;
const int out_PiezoAlarm = T8;
const int out_HB_LED = LED_BUILTIN;

unsigned long prevMillis;
unsigned long HB_period_ms = 200;

// structs to hold start/end times for scheduling.
tmElements_t tmCriticalStart;
tmElements_t tmCriticalEnd;
time_t criticalStart_time;
time_t criticalEnd_time;
time_t now_time;

bool NTPTimeSet = false;
char ntpServerNamePrimary[] = "pool.ntp.org";
char ntpServerNameSecondary[] = "time.nist.gov";
char *ntpServerName = ntpServerNamePrimary;

// forward declarations...
time_t makeTimeToday(tmElements_t &tm);

void wifi_init();
time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

void setup()
{
    Serial.begin(115200);

    // Prepare the pins to fire
    pinMode(in_GarageDoor, INPUT);
    pinMode(out_GarageLight, OUTPUT);
    pinMode(out_KitchenLight, OUTPUT);
    pinMode(out_PiezoAlarm, OUTPUT);
    pinMode(out_HB_LED, OUTPUT);

    digitalWrite(out_GarageLight, LOW);
    digitalWrite(out_KitchenLight, LOW);
    digitalWrite(out_PiezoAlarm, LOW);

    // blink the heartbeat LED a few times to indicate we're starting up wifi
    for (int i = 0; i < 3; ++i)
    {
        digitalWrite(out_HB_LED, !digitalRead(out_HB_LED));
        delay(100);
    }

    // wifi_init();

    // get timestamp for heartbeat LED
    prevMillis = millis();
}

void loop()
{
    // First let's toggle the HB LED if HB_period_ms has elapsed
    unsigned long currentMillis = millis();
    if (currentMillis - prevMillis >= HB_period_ms)
    {
        digitalWrite(out_HB_LED, !digitalRead(out_HB_LED)); // if so, change the state of the LED.
        prevMillis = currentMillis;
    }

    // Next, check if NTP has set clock time.
    // If not, set sync interval and return
    if (!NTPTimeSet)
    {
        if (timeStatus() != timeNotSet)
        {
            NTPTimeSet = true;
            Serial.println("Time now set by NTP");
            setSyncInterval(3600);
            HB_period_ms = 1000;
        }
        else
        {
            setSyncInterval(5);
            return;
        }
    }

    criticalStart_time = makeTimeToday(tmCriticalStart);
    criticalEnd_time = makeTimeToday(tmCriticalEnd);

    if (((now_time >= criticalStart_time) && (now_time <= criticalEnd_time)))
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
            force_on = false;
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

time_t secondsOfDay(const tmElements_t &tm)
{
    int32_t seconds;
    seconds = tm.Hour * SECS_PER_HOUR;
    seconds += tm.Minute * SECS_PER_MIN;
    seconds += tm.Second;
    return (time_t)seconds;
}

time_t makeTimeToday(tmElements_t &tm)
{
    // update the year, month, day to current
    tm.Year = year() - 1970;
    tm.Month = month();
    tm.Day = day();
    return makeTime(tm);
}

bool insideTimePeriod(time_t currentTime, tmElements_t &tm_start, tmElements_t &tm_end)
{
    bool insideTimePer = false;
    time_t criticalStart_time;
    time_t criticalEnd_time;
    
    // Check to see if the time period spans 2 days
    time_t start_secs = secondsOfDay(tm_start);
    time_t end_secs   = secondsOfDay(tm_end);


    if (end_secs > start_secs)
    {
        criticalStart_time = makeTimeToday(tm_start);
        criticalEnd_time   = makeTimeToday(tm_end);
    }
    else
    {
        criticalStart_time = makeTimeToday(tm_start);
        tm_end.Day++;
        criticalEnd_time   = makeTime(tm_end);
    }


    now_time = now();
    if ((now_time >= criticalStart_time) && (now_time <= criticalEnd_time))
        insideTimePer = true;

}
