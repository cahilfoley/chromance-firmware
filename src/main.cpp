#include <Arduino.h>
#include <OneButton.h>

#include "StateManager.h"
#include "config.h"

#define ENABLE_LEDS
#define ENABLE_TIME_MANAGER
// #define ENABLE_OTA
// #define ENABLE_MQTT
// #define ENABLE_SCREEN
// #define ENABLE_BENCHMARK
// #define ENABLE_BENCHMARK_BACKGROUND
// #define WAIT_FOR_SERIAL

#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_MQTT)
#include "services/WiFiManager.h"
#endif

#ifdef ENABLE_LEDS
#include <FastLED.h>

#include "animation/Graph.h"
#include "animation/Ripple.h"
#endif

#ifdef ENABLE_SCREEN
#include <U8g2lib.h>
#include <Wire.h>
#endif

#ifdef ENABLE_TIME_MANAGER
#include "services/TimeManager.h"
#endif

#ifdef ENABLE_OTA
#include "services/OTAManager.h"
#endif

#ifdef ENABLE_MQTT
#include "services/MQTTManager.h"
#endif

OneButton bootButton = OneButton(0, true, true);

// Setup LEDs and animations
#ifdef ENABLE_LEDS
#define COLOR_COUNT 6
static CRGB colors[COLOR_COUNT] = {
    CRGB(255, 0, 0),    // Red
    CRGB(0, 255, 255),  // Cyan
    CRGB(0, 255, 0),    // Green
    CRGB(255, 255, 0),  // Yellow
    CRGB(0, 0, 255),    // Blue
    CRGB(255, 0, 255)   // Magenta
};

CRGB leds[TOTAL_LEDS];  // LED buffer - each ripple writes to this

byte lastHueValue = 0;
byte lastColorIndex = 0;

// These ripples are endlessly reused so we don't need to do any memory
// management
#define numberOfRipples 30
Ripple ripples[numberOfRipples] = {
    Ripple(0),  Ripple(1),  Ripple(2),  Ripple(3),  Ripple(4),  Ripple(5),
    Ripple(6),  Ripple(7),  Ripple(8),  Ripple(9),  Ripple(10), Ripple(11),
    Ripple(12), Ripple(13), Ripple(14), Ripple(15), Ripple(16), Ripple(17),
    Ripple(18), Ripple(19), Ripple(20), Ripple(21), Ripple(22), Ripple(23),
    Ripple(24), Ripple(25), Ripple(26), Ripple(27), Ripple(28), Ripple(29),
};

unsigned long lastRandomPulse;
Node *lastAutoPulseNode;

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
  sprintf(wifiBuffer, "W: %s",
          WiFiConnection.isConnected() ? "Connected" : "Disconnected");
  u8g2.drawStr(5, 50, wifiBuffer);
#endif
  u8g2.display();
#endif
}

static void handleClick() {
  stateManager.selectNextAnimation();
  updateDisplay();
}

bool firstBackgroundLoop = true;

TaskHandle_t backgroundTask;

const unsigned long timeCheckInterval = 60000;
unsigned long nextTimeCheck = 0;

void backgroundLoop(void *parameter) {
  bootButton.attachClick(handleClick);

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

    if (firstBackgroundLoop) {
#ifdef ENABLE_SCREEN
      updateDisplay();
#endif
      firstBackgroundLoop = false;
    }

    if (millis() - stateManager.lastAnimationChange >= animationChangeTime) {
      stateManager.selectNextAnimation();
      updateDisplay();
    }

    delay(10);

#ifdef ENABLE_BENCHMARK_BACKGROUND
    Serial.print("Background benchmark: ");
    Serial.println(millis() - benchmark);
#endif
  }
}

void setup() {
  Serial.begin(115200);

#ifdef ENABLE_LEDS
  FastLED.addLeds<DOTSTAR, 5, 18, BGR>(leds, channelOffsets[0], lengths[0])
      .setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 19, 26, BGR>(leds, channelOffsets[1], lengths[1])
      .setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 25, 33, BGR>(leds, channelOffsets[2], lengths[2])
      .setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 32, 23, BGR>(leds, channelOffsets[3], lengths[3])
      .setCorrection(TypicalLEDStrip);
  FastLED.clear();
#endif

#ifdef ENABLE_SCREEN
  u8g2.begin();
#endif

#ifdef WAIT_FOR_SERIAL
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
#endif

#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_MQTT)
  wifiManager.setup();
#endif

#ifdef ENABLE_TIME_MANAGER
  timeManager.setup();
#endif

#ifdef ENABLE_OTA
  otaManager.setup();
#endif

#ifdef ENABLE_MQTT
  MQTTManager.setup();
#endif

  Serial.println("*** LET'S GOOOOO ***");

  xTaskCreatePinnedToCore(backgroundLoop, "backgroundLoop", 10000, NULL, 1,
                          &backgroundTask, 0);  // Core 0

  stateManager.lastAnimationChange = millis();
}

void loop() {
#ifdef ENABLE_BENCHMARK
  unsigned long benchmark = millis();
#endif

#ifdef ENABLE_LEDS
  stateManager.stateLock.lock();
  AnimationType currentAnimation = stateManager.animation;
  unsigned long lastAnimationChange = stateManager.lastAnimationChange;
  stateManager.stateLock.unlock();

  // Fade all dots to create trails
  if (currentAnimation != FlatRainbow) {
    fadeToBlackBy(leds, TOTAL_LEDS, 2);
  }

  for (int i = 0; i < numberOfRipples; i++) {
    if (ripples[i].state != dead) {
      ripples[i].advance(leds);
    }
  }

  FastLED.setBrightness(stateManager.brightness);
  FastLED.show();

  if (millis() - lastRandomPulse >= randomPulseTime) {
    CRGB baseColor = colors[lastColorIndex];
    lastColorIndex = (lastColorIndex + 1) % COLOR_COUNT;

    switch (currentAnimation) {
      case RandomPulses: {
        byte nodeIndex = 0;
        auto node = &graph.nodes[nodeIndex];
        bool foundStartingNode = false;
        while (!foundStartingNode) {
          nodeIndex = random8(25);
          node = &graph.nodes[nodeIndex];
          foundStartingNode = true;
          for (byte i = 0; i < numberOfBorderNodes; i++) {
            // Don't fire a pulse on one of the outer nodes - it looks boring
            if (node->index == borderNodes[i]) foundStartingNode = false;
          }

          if (node == lastAutoPulseNode) foundStartingNode = false;
        }

        lastAutoPulseNode = node;

        for (byte direction = 0; direction < 6; direction++) {
          if (node->strips[direction] != nullptr) {
            for (int rippleIndex = 0; rippleIndex < numberOfRipples;
                 rippleIndex++) {
              if (ripples[rippleIndex].state == dead) {
                ripples[rippleIndex].start(
                    node, direction, baseColor,
                    float(random8(100)) / 100.0 * .1 + .3, 3000, feisty);

                break;
              }
            }
          }
        }

        break;
      }

      case CubePulses: {
        auto node = &graph.nodes[cubeNodes[random8(numberOfCubeNodes)]];

        while (node == lastAutoPulseNode) {
          node = &graph.nodes[cubeNodes[random8(numberOfCubeNodes)]];
        }

        lastAutoPulseNode = node;

        RippleBehavior behavior =
            random8(2) ? alwaysTurnsLeft : alwaysTurnsRight;

        for (int i = 0; i < 6; i++) {
          if (node->strips[i] != nullptr) {
            for (int j = 0; j < numberOfRipples; j++) {
              if (ripples[j].state == dead) {
                ripples[j].start(node, i, baseColor, .35, 2000, behavior);

                break;
              }
            }
          }
        }
        break;
      }

      case StarburstPulses: {
        RippleBehavior behavior =
            random8(2) ? alwaysTurnsLeft : alwaysTurnsRight;

        lastAutoPulseNode = &graph.nodes[starburstNode];

        for (int i = 0; i < 6; i++) {
          for (int j = 0; j < numberOfRipples; j++) {
            if (ripples[j].state == dead) {
              ripples[j].start(
                  lastAutoPulseNode, i,
                  CHSV(((255 / 6) * (lastColorIndex + i)) % 255, 255, 255), .4,
                  1800, behavior);

              break;
            }
          }
        }
        break;
      }

      default:
        break;
    }
    lastRandomPulse = millis();
  }

  switch (currentAnimation) {
    case FlatRainbow: {
      unsigned long now = millis();
      byte hue = map(now, lastAnimationChange,
                     lastAnimationChange + animationChangeTime, 0, 255);

      // Fade the brightness in and out at the start and end of the animation
      byte value = 200;
      int animationDuration = now - lastAnimationChange;
      if (animationDuration < 1000) {
        value = map(animationDuration, 0, 1000, 0, 200);
      } else if (animationDuration > animationChangeTime - 1000) {
        value = map(animationDuration, animationChangeTime - 1000,
                    animationChangeTime + 1, 200, 0);
      }

      for (int i = 0; i < TOTAL_LEDS; i++) {
        leds[i] = CHSV(hue, 255, value);
      }

      break;
    }
  }
#endif

#ifdef ENABLE_BENCHMARK
  Serial.print("Benchmark: ");
  Serial.println(millis() - benchmark);
#endif
}
