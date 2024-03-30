#ifndef background_h
#define background_h

#include <Arduino.h>
#include <OneButton.h>

#include "StateManager.h"
#include "config.h"

#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_MQTT)
#include <WiFiManager.h>
#endif

#ifdef ENABLE_OTA
#include <OTAManager.h>
#endif

#ifdef ENABLE_LEDS
#include <FastLED.h>
#endif

#ifdef ENABLE_TIME_MANAGER
#include <TimeManager.h>
#endif

#ifdef ENABLE_SCREEN
#include <U8g2lib.h>
#include <Wire.h>
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

#ifdef ENABLE_SCREEN
// Setting up the screen to output
U8G2_SSD1306_128X64_NONAME_F_SW_I2C
u8g2(U8G2_R0, 22, 21, U8X8_PIN_NONE);
#endif

static void updateDisplay() {
#ifdef ENABLE_SCREEN
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  char animationBuffer[50];
  sprintf(animationBuffer, "A: %s", animationNames[stateManager.animation]);
  u8g2.drawStr(5, 10, animationBuffer);
  char brightnessBuffer[50];
  sprintf(brightnessBuffer, "B: %d", stateManager.brightness);
  u8g2.drawStr(5, 30, brightnessBuffer);
#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_MQTT)
  char wifiBuffer[50];
  sprintf(wifiBuffer, "W: %s", WiFiConnection.isConnected() ? "Connected" : "Disconnected");
  u8g2.drawStr(5, 50, wifiBuffer);
#endif
  u8g2.display();
#endif
}

static void handleClick() {
  stateManager.selectNextAnimation();
  updateDisplay();
}

void backgroundLoop(void *parameter) {
  bootButton.attachClick(handleClick);

#ifdef ENABLE_TIME_MANAGER
  timeManager.setup();
  printWakeUpReason();
#endif

#ifdef ENABLE_SCREEN
  u8g2.begin();
  updateDisplay();
#endif

  while (true) {
#ifdef ENABLE_BENCHMARK_BACKGROUND
    unsigned long benchmark = millis();
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

#ifdef ENABLE_OTA
    otaManager.handle();
#endif

    if (millis() - stateManager.lastAnimationChange >= animationChangeTime) {
      stateManager.selectNextAnimation();
      updateDisplay();
    }

    delay(2);

#ifdef ENABLE_BENCHMARK_BACKGROUND
    Serial.print("Background benchmark: ");
    Serial.println(millis() - benchmark);
#endif
  }
}

#endif