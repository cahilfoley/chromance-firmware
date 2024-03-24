#ifndef Ripple_h
#define Ripple_h

/**
 * WARNING: These slow things down enough to affect performance. Don't turn on
 * unless you need them!
 */
// #define DEBUG_ADVANCEMENT  // Print debug messages about ripples' movement
// #define DEBUG_RENDERING  // Print debug messages about translating logical to
// actual position

#include <FastLED.h>

#include "Graph.h"
#include "config.h"
#include "mapping.h"

enum rippleState {
  dead,
  withinNode,  // Ripple isn't drawn as it passes through a node to keep the
               // speed consistent
  travelingUpwards,
  travelingDownwards
};

enum RippleBehavior {
  weaksauce = 0,
  feisty = 1,
  angry = 2,
  alwaysTurnsRight = 3,
  alwaysTurnsLeft = 4
};

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

class Ripple {
 public:
  Ripple(int id) : rippleID(id) {}

  rippleState state = dead;
  CRGB color;

  /*
     If within a node: 0 is node, 1 is direction
     If traveling, 0 is segment, 1 is LED position from bottom
  */
  int position[2];

  Strip *strip;
  Node *node;
  byte ledIndex;
  byte direction;

  // Place the Ripple in a node
  void start(byte node, byte direction, CRGB _color, float _speed,
             unsigned long _lifespan, RippleBehavior _behavior) {
    color = _color;
    speed = _speed;
    lifespan = _lifespan;
    behavior = _behavior;

    birthday = millis();
    lastRender = millis();
    pressure = 0;
    state = withinNode;

    position[0] = node;
    position[1] = direction;

    justStarted = true;

    // Serial.print("Ripple ");
    // Serial.print(rippleID);
    // Serial.print(" starting at node ");
    // Serial.print(position[0]);
    // Serial.print(" direction ");
    // Serial.println(position[1]);
  }

  void advance(CRGB ledColors[TOTAL_LEDS]) {
    if (state == dead) return;

    unsigned long now = millis();
    unsigned long age = now - birthday;
    unsigned long timeSinceLastRender = now - lastRender;
    float timeMultiplier = fmap(timeSinceLastRender, 6, 60, 1, 10);

    pressure +=
        fmap(float(age), 0.0, float(lifespan), speed, 0.0) * timeMultiplier;

    if (pressure < 1 &&
        (state == travelingUpwards || state == travelingDownwards)) {
      // Ripple is visible but hasn't moved - render it to avoid flickering
      renderLED(ledColors, age);
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

            int sharpLeft = (position[1] + 1) % 6;
            int wideLeft = (position[1] + 2) % 6;
            int forward = (position[1] + 3) % 6;
            int wideRight = (position[1] + 4) % 6;
            int sharpRight = (position[1] + 5) % 6;

            if (behavior <= 2) {  // Semi-random aggressive turn mode
              // The more aggressive a ripple, the tighter turns it wants to
              // make. If there aren't any segments it can turn to, we need to
              // adjust its behavior.
              byte anger = behavior;

              while (newDirection < 0) {
                if (anger == 0) {
                  int forwardConnection = nodeConnections[position[0]][forward];

                  if (forwardConnection < 0) {
                    // We can't go straight ahead - we need to take a more
                    // aggressive angle
#ifdef DEBUG_ADVANCEMENT
                    Serial.println(
                        "  Can't go straight - picking more agr. path");
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
                  int leftConnection = nodeConnections[position[0]][wideLeft];
                  int rightConnection = nodeConnections[position[0]][wideRight];

                  if (leftConnection >= 0 && rightConnection >= 0) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Turning left or right at random");
#endif
                    newDirection = random(2) ? wideLeft : wideRight;
                  } else if (leftConnection >= 0) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can only turn left");
#endif
                    newDirection = wideLeft;
                  } else if (rightConnection >= 0) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can only turn right");
#endif
                    newDirection = wideRight;
                  } else {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println(
                        "  Can't make wide turn - picking more agr. path");
#endif
                    anger++;  // Can't take shallow turn - must become more
                              // aggressive
                  }
                }

                if (anger == 2) {
                  int leftConnection = nodeConnections[position[0]][sharpLeft];
                  int rightConnection =
                      nodeConnections[position[0]][sharpRight];

                  if (leftConnection >= 0 && rightConnection >= 0) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Turning left or right at random");
#endif
                    newDirection = random(2) ? sharpLeft : sharpRight;
                  } else if (leftConnection >= 0) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can only turn left");
#endif
                    newDirection = sharpLeft;
                  } else if (rightConnection >= 0) {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println("  Can only turn right");
#endif
                    newDirection = sharpRight;
                  } else {
#ifdef DEBUG_ADVANCEMENT
                    Serial.println(
                        "  Can't make tight turn - picking less agr. path");
#endif
                    anger--;  // Can't take tight turn - must become less
                              // aggressive
                  }
                }

                // Note that this can't handle some circumstances,
                // like a node with segments in nothing but the 0 and 3
                // positions. Good thing we don't have any of those!
              }
            } else if (behavior == alwaysTurnsRight) {
              for (int i = 1; i < 6; i++) {
                int possibleDirection = (position[1] + i) % 6;

                if (nodeConnections[position[0]][possibleDirection] >= 0) {
                  newDirection = possibleDirection;
                  break;
                }
              }

#ifdef DEBUG_ADVANCEMENT
              Serial.println("  Turning as rightward as possible");
#endif
            } else if (behavior == alwaysTurnsLeft) {
              for (int i = 5; i >= 1; i--) {
                int possibleDirection = (position[1] + i) % 6;

                if (nodeConnections[position[0]][possibleDirection] >= 0) {
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
            Serial.print(position[0]);
            Serial.print(" in direction ");
            Serial.println(newDirection);
#endif

            position[1] = newDirection;
          }

          position[0] =
              nodeConnections[position[0]]
                             [position[1]];  // Look up which segment we're on

#ifdef DEBUG_ADVANCEMENT
          Serial.print("  and entering segment ");
          Serial.println(position[0]);
#endif

          if (position[1] == 5 || position[1] == 0 ||
              position[1] == 1) {  // Top half of the node
#ifdef DEBUG_ADVANCEMENT
            Serial.println("  (starting at bottom)");
#endif
            state = travelingUpwards;
            position[1] = 0;  // Starting at bottom of segment
          } else {
#ifdef DEBUG_ADVANCEMENT
            Serial.println("  (starting at top)");
#endif
            state = travelingDownwards;
            position[1] =
                STRIP_LED_COUNT - 1;  // Starting at top of 14-LED-long strip
          }
          break;
        }

        case travelingUpwards: {
          position[1]++;

          if (position[1] >= STRIP_LED_COUNT) {
            // We've reached the top!
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Reached top of seg. ");
            Serial.println(position[0]);
#endif
            // Enter the new node.
            int segment = position[0];
            position[0] = segmentConnections[position[0]][0];
            for (int i = 0; i < 6; i++) {
              // Figure out from which direction the ripple is entering the
              // node. Allows us to exit in an appropriately aggressive
              // direction.
              int incomingConnection = nodeConnections[position[0]][i];
              if (incomingConnection == segment) position[1] = i;
            }
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Entering node ");
            Serial.print(position[0]);
            Serial.print(" from direction ");
            Serial.println(position[1]);
#endif
            state = withinNode;
          } else {
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Moved up to seg. ");
            Serial.print(position[0]);
            Serial.print(" LED ");
            Serial.println(position[1]);
#endif
          }
          break;
        }

        case travelingDownwards: {
          position[1]--;
          if (position[1] < 0) {
            // We've reached the bottom!
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Reached bottom of seg. ");
            Serial.println(position[0]);
#endif
            // Enter the new node.
            int segment = position[0];
            position[0] = segmentConnections[position[0]][1];
            for (int i = 0; i < 6; i++) {
              // Figure out from which direction the ripple is entering the
              // node. Allows us to exit in an appropriately aggressive
              // direction.
              int incomingConnection = nodeConnections[position[0]][i];
              if (incomingConnection == segment) position[1] = i;
            }
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Entering node ");
            Serial.print(position[0]);
            Serial.print(" from direction ");
            Serial.println(position[1]);
#endif
            state = withinNode;
          } else {
#ifdef DEBUG_ADVANCEMENT
            Serial.print("  Moved down to seg. ");
            Serial.print(position[0]);
            Serial.print(" LED ");
            Serial.println(position[1]);
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
      position[0] = position[1] = pressure = age = 0;
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

  void renderLED(CRGB ledColors[TOTAL_LEDS], unsigned long age) {
    int stripIndex = position[0];
    int channel = ledAssignments[stripIndex][0];
    int channelLEDOffset = ledAssignments[stripIndex][1];
    int stripStart = channelOffsets[channel] + ledAssignments[stripIndex][1];
    int stripEnd = channelOffsets[channel] + ledAssignments[stripIndex][2];

    int led = map(position[1], 0, 13, stripEnd, stripStart);

#ifdef DEBUG_RENDERING
    Serial.print("Position: ");
    Serial.print(position[0]);
    Serial.print(", ");
    Serial.print(position[1]);
    Serial.print(" -> ");
    Serial.print("Rendering ripple position (stripIndex: ");
    Serial.print(stripIndex);
    Serial.print(", stripOffset: ");
    Serial.print(stripOffset);
    Serial.print(", ledIndex: ");
    Serial.println(ledIndex);
#endif

    CRGB currentColour = ledColors[led];

    ledColors[led].setRGB(
        byte(min(255, max(0, int(fmap(float(age), 0.0, float(lifespan), color.r,
                                      0.0)) +
                                 currentColour.r))),
        byte(min(255, max(0, int(fmap(float(age), 0.0, float(lifespan), color.g,
                                      0.0)) +
                                 currentColour.g))),
        byte(min(255, max(0, int(fmap(float(age), 0.0, float(lifespan), color.b,
                                      0.0)) +
                                 currentColour.b))));

    int nextLED = led + 1;
    if (pressure < 1 && nextLED % STRIP_LED_COUNT != 0) {
      CRGB nextCurrentColour = ledColors[nextLED];

      // If we are partially into the next LED we can partially render it
      ledColors[nextLED].setRGB(
          byte(min(255, max(0, int(fmap(float(age), 0.0, float(lifespan),
                                        color.r * pressure, 0.0) +
                                   nextCurrentColour.r)))),
          byte(min(255, max(0, int(fmap(float(age), 0.0, float(lifespan),
                                        color.g * pressure, 0.0) +
                                   nextCurrentColour.g)))),
          byte(min(255, max(0, int(fmap(float(age), 0.0, float(lifespan),
                                        color.b * pressure, 0.0) +
                                   nextCurrentColour.b)))));
    }

#ifdef DEBUG_RENDERING
    Serial.print("Rendering ripple position (");
    Serial.print(position[0]);
    Serial.print(',');
    Serial.print(position[1]);
    Serial.print(") at Strip ");
    Serial.print(strip);
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

#endif
