#ifndef CubePulses_h
#define CubePulses_h

#include "animation/base/RippleBasedAnimation.h"
#include "config.h"
#include "system/Graph.h"

class CubePulses : public RippleBasedAnimation {
 public:
  CubePulses() : RippleBasedAnimation("Cube Pulses", CUBE_PULSES) {}

  void preRender(CRGB leds[TOTAL_LEDS]) {
    RippleBasedAnimation::preRender(leds);
    unsigned long now = millis();
    if (now - lastRandomPulse <= randomPulseInterval) return;

    lastRandomPulse = now;
    lastColorIndex = (lastColorIndex + 1) % colorCount;
    auto baseColor = &colors[lastColorIndex];

    byte loopProtection = 0;
    auto node = &graph.nodes[cubeNodes[random8(numberOfCubeNodes)]];
    while (node == lastAutoPulseNode) {
      node = &graph.nodes[cubeNodes[random8(numberOfCubeNodes)]];
    }

    lastAutoPulseNode = node;

    RippleBehavior behavior = random8(2) ? alwaysTurnsLeft : alwaysTurnsRight;

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
  };

 private:
  Node *lastAutoPulseNode;
  byte lastColorIndex = 0;
  unsigned long lastRandomPulse = 0;
};

#endif