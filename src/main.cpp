#include <Arduino.h>
#include <OneButton.h>

#include "StateManager.h"
#include "background.h"
#include "config.h"

#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_MQTT)
#include <WiFiManager.h>
#endif

#ifdef ENABLE_LEDS
#include <FastLED.h>

#include "animation/CubePulses.h"
#include "animation/FlatRainbow.h"
#include "animation/RandomPulses.h"
#include "animation/StarburstPulses.h"
#include "animation/base/Ripple.h"
#include "system/Graph.h"
#endif

#ifdef ENABLE_OTA
#include <OTAManager.h>
#endif

#ifdef ENABLE_MQTT
#include <MQTTManager.h>
#endif

#ifdef ENABLE_TIME_MANAGER
void goToSleep() {
  int sleepSeconds = min(stateManager.sleepTimeSeconds, 60 * 60);  // Cap sleep time at 1 hour
  Serial.print("Going to sleep for ");
  Serial.print(sleepSeconds);
  Serial.println(" seconds");

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

// Setup LEDs and animations
#ifdef ENABLE_LEDS
CRGB leds[TOTAL_LEDS];  // LED buffer - each ripple writes to this
#endif

void setup() {
  Serial.begin(115200);

#ifdef ENABLE_LEDS
  FastLED.addLeds<DOTSTAR, 5, 18, BGR>(leds, channelOffsets[0], lengths[0]).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 19, 26, BGR>(leds, channelOffsets[1], lengths[1]).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 25, 33, BGR>(leds, channelOffsets[2], lengths[2]).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 32, 23, BGR>(leds, channelOffsets[3], lengths[3]).setCorrection(TypicalLEDStrip);
  FastLED.clear();
#endif

#ifdef WAIT_FOR_SERIAL
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
#endif

#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_MQTT)
  wifiManager.setup();
#endif

#ifdef ENABLE_OTA
  otaManager.setup();
#endif

#ifdef ENABLE_MQTT
  MQTTManager.setup();
#endif

  Serial.println("*** LET'S GOOOOO ***");

  xTaskCreatePinnedToCore(backgroundLoop, "backgroundLoop", 10000, NULL, 1, &backgroundTask, 0);  // Core 0

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

#ifdef ENABLE_OTA
  otaManager.handle();
#endif

#ifdef ENABLE_LEDS
  auto animation = stateManager.animation;

  animation->preRender(leds);
  animation->render(leds);

  FastLED.setBrightness(animation->adjustBrightness(stateManager.brightness));
  FastLED.show();

  animation->postRender(leds);
#endif

#ifdef ENABLE_BENCHMARK
  Serial.print("Benchmark: ");
  Serial.println(millis() - benchmark);
#endif
}
