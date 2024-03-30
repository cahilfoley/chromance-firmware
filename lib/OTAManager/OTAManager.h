#ifndef OTAManager_h
#define OTAManager_h

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>

class OTAManager {
 public:
  void setup() {
    // Hostname defaults to esp3232-[MAC]
    ArduinoOTA.setHostname("chromance-esp32");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    ArduinoOTA
        .onStart([]() {
          String type;
          if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
          else
            type = "filesystem";
          Serial.println("Start updating " + type);
        })
        .onEnd([]() { Serial.println("\nEnd"); })
        .onProgress([](unsigned int progress, unsigned int total) {
          Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
          Serial.printf("Error[%u]: ", error);
          if (error == OTA_AUTH_ERROR)
            Serial.println("Auth Failed");
          else if (error == OTA_BEGIN_ERROR)
            Serial.println("Begin Failed");
          else if (error == OTA_CONNECT_ERROR)
            Serial.println("Connect Failed");
          else if (error == OTA_RECEIVE_ERROR)
            Serial.println("Receive Failed");
          else if (error == OTA_END_ERROR)
            Serial.println("End Failed");
        });

    ArduinoOTA.begin();

    Serial.println("OTA updates are ready");
  }

  void handle() { ArduinoOTA.handle(); }
};

OTAManager otaManager;

#endif