#ifndef TimeManager_h
#define TimeManager_h

#include <WiFi.h>

#include "config.h"
#include "time.h"

class TimeManager {
 public:
  void setup() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    Serial.println("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 60 * 60 * 2) {
      delay(500);
      Serial.print(".");
      now = time(nullptr);
    }

    localtime_r(&now, &timeinfo);
    Serial.println("Time successfully synced");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }

  void printTime() {
    struct tm timeNow = getCurrentLocalTime();
    Serial.print("The current local time is: ");
    Serial.println(&timeNow, "%A, %B %d %Y %H:%M:%S");
  }

  struct tm getCurrentLocalTime() {
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);
    return timeinfo;
  }

 private:
  struct tm timeinfo;
};

TimeManager timeManager;

#endif