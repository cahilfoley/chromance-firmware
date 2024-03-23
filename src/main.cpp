#include <Arduino.h>
#include <OneButton.h>

#define ENABLE_LEDS
// #define ENABLE_OTA
// #define ENABLE_MQTT
// #define ENABLE_SCREEN
#define ENABLE_BENCHMARK

#if defined(ENABLE_OTA) || defined(ENABLE_MQTT)
#include "WiFiManager.h"
#endif

#ifdef ENABLE_LEDS
#include <Adafruit_DotStar.h>
#include <SPI.h>

#include "mapping.h"
#include "ripple.h"
#endif

#ifdef ENABLE_SCREEN
#include <U8g2lib.h>
#include <Wire.h>
#endif

#ifdef ENABLE_OTA
#include "OTAManager.h"
#endif

#ifdef ENABLE_MQTT
#include "MQTTManager.h"
#endif

// Auto pulse configuration
enum AutoPulseType { RandomPulses = 0, CubePulses = 1, StarburstPulses = 2 };
const char* autoPulseNames[] = {
    "Random Pulses",
    "Cube Pulses",
    "Starburst Pulses",
};

#define randomPulsesEnabled true     // Fire random pulses from random nodes
#define cubePulsesEnabled true       // Draw cubes at random nodes
#define starburstPulsesEnabled true  // Draw starbursts

#define randomPulseTime 2000  // Fire a random pulse every (this many) ms

byte numberOfAutoPulseTypes =
    randomPulsesEnabled + cubePulsesEnabled + starburstPulsesEnabled;
AutoPulseType currentAutoPulseType = RandomPulses;
#define autoPulseChangeTime 30000
unsigned long lastAutoPulseChange;

OneButton bootButton = OneButton(0, true, true);

// Setup LEDs and animations
#ifdef ENABLE_LEDS
int lengths[] = {154, 168, 84,
                 154};  // Strips are different lengths because I am a dumb

Adafruit_DotStar strip0(lengths[0], 5, 18, DOTSTAR_GRB);
Adafruit_DotStar strip1(lengths[1], 19, 26, DOTSTAR_GRB);
Adafruit_DotStar strip2(lengths[2], 25, 33, DOTSTAR_GRB);
Adafruit_DotStar strip3(lengths[3], 32, 23, DOTSTAR_GRB);

Adafruit_DotStar strips[4] = {strip0, strip1, strip2, strip3};

byte ledColors[40][14][3];  // LED buffer - each ripple writes to this, then we
                            // write this to the strips
float decay = 0.97;  // Multiply all LED's by this amount each tick to create
                     // fancy fading tails

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
byte lastAutoPulseNode = 255;

#endif

#ifdef ENABLE_SCREEN
// Setting up the screen to output
U8G2_SSD1306_128X64_NONAME_F_SW_I2C
u8g2(U8G2_R0, 22, 21, U8X8_PIN_NONE);
#endif

static void selectNextAnimation() {
  while (true) {
    currentAutoPulseType = static_cast<AutoPulseType>(
        (static_cast<int>(currentAutoPulseType) + 1) % numberOfAutoPulseTypes);
    if (currentAutoPulseType == RandomPulses && randomPulsesEnabled) break;
    if (currentAutoPulseType == CubePulses && cubePulsesEnabled) break;
    if (currentAutoPulseType == StarburstPulses && starburstPulsesEnabled)
      break;
  }

  lastAutoPulseChange = millis();

#ifdef ENABLE_SCREEN
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  char buffer[128];  // Buffer to hold the formatted string
  sprintf(buffer, "Animation: %s", autoPulseNames[currentAutoPulseType]);
  u8g2.drawStr(10, 10, buffer);
  u8g2.drawStr(10, 30, "Play Mode: Automatic");
  u8g2.sendBuffer();
#endif
}

static void handleClick() { selectNextAnimation(); };

void setup() {
  Serial.begin(115200);

  bootButton.attachClick(handleClick);

#ifdef ENABLE_SCREEN
  u8g2.begin();
#endif

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

#if defined(ENABLE_OTA) || defined(ENABLE_MQTT)
  WiFiConnection.setup();
#endif

#ifdef ENABLE_OTA
  setupOTA();
#endif

#ifdef ENABLE_MQTT
  MQTTManager.setup();
#endif

  Serial.println("*** LET'S GOOOOO ***");

#ifdef ENABLE_LEDS
  for (int i = 0; i < 4; i++) {
    strips[i].begin();
    strips[i].show();
  }
#endif
}

void loop() {
#ifdef ENABLE_BENCHMARK
  unsigned long benchmark = millis();
#endif

  bootButton.tick();

#ifdef ENABLE_OTA
  handleOTA();
#endif

#ifdef ENABLE_LEDS
  // Fade all dots to create trails
  for (int strip = 0; strip < 40; strip++) {
    for (int led = 0; led < 14; led++) {
      for (int i = 0; i < 3; i++) {
        ledColors[strip][led][i] *= decay;
      }
    }
  }

  for (int i = 0; i < numberOfRipples; i++) {
    ripples[i].advance(ledColors);
  }

  // ~1ms of loop time here
  for (int segment = 0; segment < 40; segment++) {
    for (int fromBottom = 0; fromBottom < 14; fromBottom++) {
      int strip = ledAssignments[segment][0];
      int led = round(fmap(fromBottom, 0, 13, ledAssignments[segment][2],
                           ledAssignments[segment][1]));
      strips[strip].setPixelColor(led, ledColors[segment][fromBottom][0],
                                  ledColors[segment][fromBottom][1],
                                  ledColors[segment][fromBottom][2]);
    }
  }

  for (int i = 0; i < 4; i++) strips[i].show();
#endif

  // Negligable performance impact
  if (numberOfAutoPulseTypes > 1 &&
      millis() - lastAutoPulseChange >= autoPulseChangeTime) {
    selectNextAnimation();
  }

#ifdef ENABLE_LEDS
  if (numberOfAutoPulseTypes && millis() - lastRandomPulse >= randomPulseTime) {
    unsigned int baseColor = random(0xFFFF);

    switch (currentAutoPulseType) {
      case RandomPulses: {
        int node = 0;
        bool foundStartingNode = false;
        while (!foundStartingNode) {
          node = random(25);
          foundStartingNode = true;
          for (int i = 0; i < numberOfBorderNodes; i++) {
            // Don't fire a pulse on one of the outer nodes - it looks boring
            if (node == borderNodes[i]) foundStartingNode = false;
          }

          if (node == lastAutoPulseNode) foundStartingNode = false;
        }

        lastAutoPulseNode = node;

        for (int direction = 0; direction < 6; direction++) {
          if (nodeConnections[node][direction] >= 0) {
            for (int rippleIndex = 0; rippleIndex < numberOfRipples;
                 rippleIndex++) {
              if (ripples[rippleIndex].state == dead) {
                ripples[rippleIndex].start(
                    node, direction,
                    // strip0.ColorHSV(baseColor + (0xFFFF / 6) * i, 255, 255),
                    strip0.ColorHSV(baseColor, 255, 255),
                    float(random(100)) / 100.0 * .2 + .5, 3000, feisty);

                break;
              }
            }
          }
        }
        break;
      }

      case CubePulses: {
        int node = cubeNodes[random(numberOfCubeNodes)];

        while (node == lastAutoPulseNode)
          node = cubeNodes[random(numberOfCubeNodes)];

        lastAutoPulseNode = node;

        RippleBehavior behavior =
            random(2) ? alwaysTurnsLeft : alwaysTurnsRight;

        for (int i = 0; i < 6; i++) {
          if (nodeConnections[node][i] >= 0) {
            for (int j = 0; j < numberOfRipples; j++) {
              if (ripples[j].state == dead) {
                ripples[j].start(
                    node, i,
                    // strip0.ColorHSV(baseColor + (0xFFFF / 6) * i, 255, 255),
                    strip0.ColorHSV(baseColor, 255, 255), .5, 2000, behavior);

                break;
              }
            }
          }
        }
        break;
      }

      case StarburstPulses: {
        RippleBehavior behavior =
            random(2) ? alwaysTurnsLeft : alwaysTurnsRight;

        lastAutoPulseNode = starburstNode;

        for (int i = 0; i < 6; i++) {
          for (int j = 0; j < numberOfRipples; j++) {
            if (ripples[j].state == dead) {
              ripples[j].start(
                  starburstNode, i,
                  strip0.ColorHSV(baseColor + (0xFFFF / 6) * i, 255, 255), .7,
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
#endif

#ifdef ENABLE_BENCHMARK
  Serial.print("Benchmark: ");
  Serial.println(millis() - benchmark);
#endif
}
