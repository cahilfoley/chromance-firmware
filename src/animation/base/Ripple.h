#ifndef Ripple_h
#define Ripple_h

/**
 * WARNING: These slow things down enough to affect performance. Don't turn on
 * unless you need them!
 */
// #define DEBUG_ADVANCEMENT  // Print debug messages about ripples' movement
// #define DEBUG_RENDERING  // Print debug messages about translating logical to actual position

#include <FastLED.h>

#include "config.h"
#include "helpers.h"
#include "system/Graph.h"
#include "system/mapping.h"

enum rippleState {
  dead,
  withinNode,  // Ripple isn't drawn as it passes through a node to keep the
               // speed consistent
  travelingUpwards,
  travelingDownwards
};

enum RippleBehavior { weaksauce = 0, feisty = 1, angry = 2, alwaysTurnsRight = 3, alwaysTurnsLeft = 4 };

class Ripple {
 public:
  Ripple(int id) : rippleID(id) {}

  rippleState state = dead;
  const CRGB *color;

  Strip *strip;
  Node *node;
  int ledIndex;
  byte direction;

  // Place the Ripple in a node
  void start(Node *node, byte direction, const CRGB *_color, float _speed, unsigned long _lifespan,
             RippleBehavior _behavior) {
    color = _color;
    speed = _speed;
    lifespan = _lifespan;
    behavior = _behavior;

    birthday = lastRender = millis();
    pressure = 0;
    state = withinNode;

    ledIndex = 0;
    this->node = node;
    this->direction = direction;

    justStarted = true;
  }

  void advance(CRGB ledColors[TOTAL_LEDS]) {
    if (state == dead) return;

    unsigned long now = millis();
    unsigned long age = now - birthday;
    unsigned long timeSinceLastRender = now - lastRender;

    // If we are suddenly older than our lifespan, we need to die, probably happened because of a non-ripple based
    // animation running for a while
    if (age > lifespan + 1000) {
      state = dead;
      return;
    }

    float timeMultiplier = fmap(timeSinceLastRender, 6, 60, 1, 10);

    float previousPressure = pressure;
    pressure += fmap(float(age), 0.0, float(lifespan), speed, 0.0) * timeMultiplier;

    if (pressure < 1 && (state == travelingUpwards || state == travelingDownwards)) {
      // Ripple is visible but hasn't moved - render it to avoid flickering
      // Need to provide the previous pressure that we applied so we only add the difference
      renderLED(ledColors, age, true, previousPressure);
    }

    while (pressure >= 1) {
#ifdef DEBUG_ADVANCEMENT
      Serial.print("Ripple ");
      Serial.print(rippleID);
      Serial.println(" advancing:");
#endif

      switch (state) {
        case withinNode: {
          if (justStarted) {
            justStarted = false;
          } else {
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Picking direction out of node ");
            Serial.print(position[0]);
            Serial.print(" with agr. ");
            Serial.println(behavior);
#endif

            int newDirection = -1;

            int sharpLeft = (direction + 1) % 6;
            int wideLeft = (direction + 2) % 6;
            int forward = (direction + 3) % 6;
            int wideRight = (direction + 4) % 6;
            int sharpRight = (direction + 5) % 6;

            if (behavior <= 2) {
              // Semi-random aggressive turn mode
              // The more aggressive a ripple, the tighter turns it wants to
              // make. If there aren't any segments it can turn to, we need to
              // adjust its behavior.
              byte anger = behavior;

              while (newDirection < 0) {
                if (anger == 0) {
                  auto forwardConnection = node->strips[forward];

                  if (forwardConnection == nullptr) {
                    // We can't go straight ahead - we need to take a more
                    // aggressive angle
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can't go straight - picking more agr. path");
#endif
                    anger++;
                  } else {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Going forward");
#endif
                    newDirection = forward;
                  }
                }

                if (anger == 1) {
                  auto leftConnection = node->strips[wideLeft];
                  auto rightConnection = node->strips[wideRight];

                  if (leftConnection != nullptr && rightConnection != nullptr) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Turning left or right at random");
#endif
                    newDirection = random8(2) ? wideLeft : wideRight;
                  } else if (leftConnection != nullptr) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can only turn left");
#endif
                    newDirection = wideLeft;
                  } else if (rightConnection != nullptr) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can only turn right");
#endif
                    newDirection = wideRight;
                  } else {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can't make wide turn - picking more agr. path");
#endif
                    // Can't take shallow turn - must become more aggressive
                    anger++;
                  }
                }

                if (anger == 2) {
                  auto leftConnection = node->strips[sharpLeft];
                  auto rightConnection = node->strips[sharpRight];

                  if (leftConnection != nullptr && rightConnection != nullptr) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Turning left or right at random");
#endif
                    newDirection = random8(2) ? sharpLeft : sharpRight;
                  } else if (leftConnection != nullptr) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can only turn left");
#endif
                    newDirection = sharpLeft;
                  } else if (rightConnection != nullptr) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can only turn right");
#endif
                    newDirection = sharpRight;
                  } else {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can't make tight turn - picking less agr. path");
#endif
                    // Can't take tight turn - must become less aggressive
                    anger--;
                  }
                }

                // Note that this can't handle some circumstances,
                // like a node with segments in nothing but the 0 and 3
                // positions. Good thing we don't have any of those!
              }
            } else if (behavior == alwaysTurnsRight) {
              for (int i = 1; i < 6; i++) {
                int possibleDirection = (direction + i) % 6;

                if (node->strips[possibleDirection] != nullptr) {
                  newDirection = possibleDirection;
                  break;
                }
              }

#ifdef DEBUG_ADVANCEMENT
              Serial.println("  Turning as rightward as possible");
#endif
            } else if (behavior == alwaysTurnsLeft) {
              for (int i = 5; i >= 1; i--) {
                int possibleDirection = (direction + i) % 6;

                if (node->strips[possibleDirection] != nullptr) {
                  newDirection = possibleDirection;
                  break;
                }
              }

#ifdef DEBUG_ADVANCEMENT
              Serial.println("  Turning as leftward as possible");
#endif
            }

#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Leaving node ");
            Serial.print(node->index);
            Serial.print(" in direction ");
            Serial.println(newDirection);
#endif

            direction = newDirection;
          }

          strip = node->strips[direction];

#ifdef DEBUG_ADVANCEMENT
          Serial.print("  and entering segment ");
          Serial.println(strips->index);
#endif

          if (direction == 5 || direction == 0 || direction == 1) {  // Top half of the node
#ifdef DEBUG_ADVANCEMENT
            Serial.println("  (starting at bottom)");
#endif
            state = travelingUpwards;
            ledIndex = 0;  // Starting at bottom of segment
          } else {
#ifdef DEBUG_ADVANCEMENT
            Serial.println("  (starting at top)");
#endif
            state = travelingDownwards;
            ledIndex = strip->length - 1;  // Starting at top
          }
          break;
        }

        case travelingUpwards: {
          ledIndex++;

          if (ledIndex >= strip->length) {
            // We've reached the top!
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Reached top of seg. ");
            Serial.println(strip->index);
#endif
            // Enter the new node.
            node = strip->topNode;
            direction = (direction + 3) % 6;  // Reverse direction
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Entering node ");
            Serial.print(node->index);
            Serial.print(" from direction ");
            Serial.println(direction);
#endif
            state = withinNode;
          } else {
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Moved up to seg. ");
            Serial.print(strip->index);
            Serial.print(" LED ");
            Serial.println(ledIndex);
#endif
          }
          break;
        }

        case travelingDownwards: {
          ledIndex--;
          if (ledIndex < 0) {
            // We've reached the bottom!
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Reached bottom of seg. ");
            Serial.println(strip->index);
#endif
            // Enter the new node.
            node = strip->bottomNode;
            direction = (direction + 3) % 6;  // Reverse direction
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Entering node ");
            Serial.print(node->index);
            Serial.print(" from direction ");
            Serial.println(direction);
#endif
            state = withinNode;
          } else {
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Moved down to seg. ");
            Serial.print(strip->index);
            Serial.print(" LED ");
            Serial.println(ledIndex);
#endif
          }
          break;
        }

        default:
          break;
      }

      pressure -= 1;

      if (state == travelingUpwards || state == travelingDownwards) {
        // Ripple is visible - render it
        renderLED(ledColors, age);
      }
    }

#ifdef DEBUG_ADVANCEMENT
    Serial.print("  Age is now ");
    Serial.print(age);
    Serial.print('/');
    Serial.println(lifespan);
#endif

    if (lifespan && age >= lifespan) {
      // We dead
#ifdef DEBUG_ADVANCEMENT
      Serial.println("  Lifespan is up! Ripple is dead.");
#endif
      state = dead;
      direction = pressure = age = 0;
    }

    lastRender = millis();
  }

 private:
  float speed;             // Each loop, ripples move this many LED's.
  unsigned long lifespan;  // The ripple stops after this many milliseconds
  unsigned long lastRender;

  RippleBehavior behavior;

  bool justStarted = false;

  float pressure;          // When Pressure reaches 1, ripple will move
  unsigned long birthday;  // Used to track age of ripple

  byte rippleID;  // Used to identify this ripple in debug output

  void renderLED(CRGB ledColors[TOTAL_LEDS], unsigned long age, bool notMoved = false, float previousPressure = 1.0) {
    float ageBasedProportion = 1.0 - min(float(age) / float(lifespan), float(1.0));

    strip->applyColorToLED(ledColors, ledIndex, color,
                           ageBasedProportion * (notMoved ? (pressure - previousPressure + 0.05) : 1));

    // int nextLED = ledIndex + (state == travelingUpwards ? 1 : -1);
    // if (pressure < 1 && nextLED >= 0 && nextLED < strip->length) {
    //   // If we are partially into the next LED we can partially render it
    //   strip->applyColorToLED(ledColors, nextLED, color, pressure * ageBasedProportion);
    // }

#ifdef DEBUG_RENDERING
    Serial.print("Rendering ripple position (");
    Serial.print(strip->index);
    Serial.print(',');
    Serial.print(ledIndex);
    Serial.print(") at Strip ");
    Serial.print(strip->index);
    Serial.print(", LED ");
    Serial.print(led);
    Serial.print(", color 0x");
    for (int i = 0; i < 3; i++) {
      if (ledColors[position[0]][position[1]][i] <= 0x0F) Serial.print('0');
      Serial.print(ledColors[position[0]][position[1]][i], HEX);
    }
    Serial.println();
#endif
  }
};

// These ripples are endlessly reused so we don't need to do any memory
// management
#define numberOfRipples 30
Ripple ripples[numberOfRipples] = {
    Ripple(0),  Ripple(1),  Ripple(2),  Ripple(3),  Ripple(4),  Ripple(5),  Ripple(6),  Ripple(7),
    Ripple(8),  Ripple(9),  Ripple(10), Ripple(11), Ripple(12), Ripple(13), Ripple(14), Ripple(15),
    Ripple(16), Ripple(17), Ripple(18), Ripple(19), Ripple(20), Ripple(21), Ripple(22), Ripple(23),
    Ripple(24), Ripple(25), Ripple(26), Ripple(27), Ripple(28), Ripple(29),
};

#endif
