#include <Arduino.h>
#include <OneButton.h>

#include "StateManager.h"
#include "background.h"
#include "config.h"
#include "secrets.h"

#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_HOME_ASSISTANT)
#include <WiFiManager.h>
#endif

#ifdef ENABLE_LEDS
#include <FastLED.h>

// Include all required animations
#include "animation/CubePulses.h"
#include "animation/FlatRainbow.h"
#include "animation/RainbowWave.h"
#include "animation/RandomPulses.h"
#include "animation/ReadingMode.h"
#include "animation/StarTwinkle.h"
#include "animation/StarburstPulses.h"
#include "animation/base/Ripple.h"
#include "system/Graph.h"
#endif

#ifdef ENABLE_OTA
#include <OTAManager.h>
#endif

#ifdef ENABLE_TIME_MANAGER
void goToSleep() {
  int sleepSeconds =
      max(min(abs(stateManager.sleepTimeSeconds), 60 * 60), 30);  // Cap sleep time between 30 secs and 1 hour
  Serial.print("Going to sleep for ");
  Serial.print(sleepSeconds);
  Serial.println(" seconds");
#ifdef ENABLE_SCREEN
  displayManager.showMessage("Sleeping...");
#endif

  wifiManager.disconnect();
#ifdef ENABLE_LEDS
  FastLED.clear(true);
  FastLED.show();
#endif
  Serial.flush();

  // Enable the boot button to wake up the device instead of the timer
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
  // Good night
  esp_deep_sleep((u64_t)sleepSeconds * (u64_t)US_TO_S_FACTOR);
}
#endif

// A reference to the background thread
TaskHandle_t backgroundTask;

void setup() {
  disableCore0WDT();
  disableCore1WDT();

  Serial.begin(115200);

#ifdef ENABLE_LEDS
  FastLED.addLeds<DOTSTAR, 13, 12, BGR>(leds, channelOffsets[0], lengths[0]).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 14, 27, BGR>(leds, channelOffsets[1], lengths[1]).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 26, 25, BGR>(leds, channelOffsets[2], lengths[2]).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 33, 32, BGR>(leds, channelOffsets[3], lengths[3]).setCorrection(TypicalLEDStrip);
  FastLED.clear();
#endif

#ifdef WAIT_FOR_SERIAL
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
#endif

#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_HOME_ASSISTANT)
  wifiManager.setup(wifiSSID, wifiPassword);
#endif

#ifdef ENABLE_HOME_ASSISTANT
  haManager.setup(mqttBroker, mqttPort, mqttUsername, mqttPassword);
#endif

#ifdef ENABLE_OTA
  otaManager.setup();
#endif

#ifdef DEVELOPMENT_BUILD
  Serial.println("*** DEVELOPMENT BUILD ***");
  Serial.println("Development build provides enhanced debugging information");
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("Available animations: ");
  Serial.println(ANIMATION_COUNT);
#else
  Serial.println("*** LET'S GOOOOO ***");
#endif

  xTaskCreatePinnedToCore(backgroundLoop, "backgroundLoop", 10000, NULL, 1, &backgroundTask, 0);

  stateManager.lastAnimationChange = millis();
}

void loop() {
#ifdef ENABLE_BENCHMARK
  unsigned long benchmark = millis();
#endif

#ifdef ENABLE_TIME_MANAGER
  if (stateManager.canSleep) {
    goToSleep();
  }
#endif

#ifdef ENABLE_HOME_ASSISTANT
  haManager.loop();
#endif

#ifdef ENABLE_OTA
  otaManager.handle();
#endif

#ifdef ENABLE_LEDS
  if (stateManager.enabled) {
    auto animation = stateManager.animation;

    // Track the last brightness to detect changes
    static byte lastBrightness = stateManager.brightness;
    bool brightnessChanged = (lastBrightness != stateManager.brightness);
    lastBrightness = stateManager.brightness;

#ifdef DEVELOPMENT_BUILD
    // In development mode, add some basic performance monitoring
    unsigned long renderStart = millis();
#endif

    animation->preRender(leds);
    bool needsUpdate = animation->render(leds);

    // Update LEDs if animation needs update OR if brightness has changed
    if (needsUpdate || brightnessChanged) {
      FastLED.setBrightness(animation->adjustBrightness(stateManager.brightness));
      FastLED.show();

#ifdef DEVELOPMENT_BUILD
      if (brightnessChanged && !needsUpdate) {
        Serial.println("Updating LEDs due to brightness change only");
      }
#endif
    }

    animation->postRender(leds);

#ifdef DEVELOPMENT_BUILD
    // Only log timing in development builds
    unsigned long renderTime = millis() - renderStart;
    // Log render time every second to avoid flooding Serial
    static unsigned long lastRenderLog = 0;
    if (millis() - lastRenderLog > 1000) {
      Serial.print("Animation render time (ms): ");
      Serial.println(renderTime);
      Serial.print("Current animation: ");
      Serial.println(animation->name);
      lastRenderLog = millis();
    }
#endif
  } else {
    FastLED.clear(true);
    delay(50);
  }
#endif

#ifdef ENABLE_BENCHMARK
  Serial.print("Benchmark: ");
  Serial.println(millis() - benchmark);
#endif
}
