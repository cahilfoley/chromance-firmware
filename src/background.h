#ifndef background_h
#define background_h

#include <Arduino.h>
#include <OneButton.h>

#include "StateManager.h"
#include "config.h"
#include "secrets.h"

#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_MQTT)
#include <WiFiManager.h>
#endif

#ifdef ENABLE_LEDS
#include <FastLED.h>
#endif

#ifdef ENABLE_TIME_MANAGER
#include <TimeManager.h>
#endif

#ifdef ENABLE_SCREEN
#include <DisplayManager.h>
#endif

#ifdef ENABLE_HOME_ASSISTANT
#include "managers/HAManager.h"
#endif

#define US_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */

OneButton bootButton = OneButton(0, true, true);

#ifdef ENABLE_TIME_MANAGER
const unsigned long timeCheckInterval = 60000;
unsigned long nextTimeCheck = 0;

/**
 * Method to print the reason by which ESP32 has been awaken from sleep
 */
void printWakeUpReason() {
  auto wakeUpReason = esp_sleep_get_wakeup_cause();

  switch (wakeUpReason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeUpReason);
      break;
  }
}
#endif

static void handleClick() { stateManager.selectNextAnimation(); }

void backgroundLoop(void *parameter) {
  bootButton.attachClick(handleClick);

#ifdef ENABLE_HOME_ASSISTANT
  haManager.setup(mqttBroker, mqttPort, mqttUsername, mqttPassword);
#endif

#ifdef ENABLE_TIME_MANAGER
  timeManager.setup();
  printWakeUpReason();
#endif

#ifdef ENABLE_SCREEN
  displayManager.setup();
#endif

  while (true) {
#ifdef ENABLE_BENCHMARK_BACKGROUND
    unsigned long benchmark = millis();
#endif

#ifdef ENABLE_HOME_ASSISTANT
    haManager.loop();
#endif

    bootButton.tick();

#ifdef ENABLE_TIME_MANAGER
    unsigned long now = millis();
    if (now >= nextTimeCheck) {
      auto time = timeManager.getCurrentLocalTime();
      stateManager.updateBrightnessFromTime(time);
      nextTimeCheck = now + timeCheckInterval;
    }
#endif

    if (millis() - stateManager.lastAnimationChange >= animationChangeTime) {
      stateManager.selectNextAnimation();
    }

    delay(1);

#ifdef ENABLE_BENCHMARK_BACKGROUND
    Serial.print("Background benchmark: ");
    Serial.println(millis() - benchmark);
#endif
  }
}

#endif