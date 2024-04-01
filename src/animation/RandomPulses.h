#ifndef RandomPulses_h
#define RandomPulses_h

#include "animation/base/RippleBasedAnimation.h"
#include "config.h"

class RandomPulses : public RippleBasedAnimation {
 public:
  RandomPulses() : RippleBasedAnimation("Random Pulses") {}

  void preRender(CRGB leds[TOTAL_LEDS]) {
    RippleBasedAnimation::preRender(leds);

    unsigned long now = millis();
    if (now - lastRandomPulse <= randomPulseInterval) return;

    lastRandomPulse = now;
    lastColorIndex = (lastColorIndex + 1) % colorCount;
    auto baseColor = &colors[lastColorIndex];

    auto node = &graph.nodes[random(NODE_COUNT)];
    byte loopProtection = 0;
    while ((node == lastAutoPulseNode || node->borderNode) && loopProtection++ < 50) {
      node = &graph.nodes[random8(NODE_COUNT)];
    }

    lastAutoPulseNode = node;

    for (byte direction = 0; direction < 6; direction++) {
      if (node->strips[direction] != nullptr) {
        for (int rippleIndex = 0; rippleIndex < numberOfRipples; rippleIndex++) {
          if (ripples[rippleIndex].state == dead) {
            ripples[rippleIndex].start(node, direction, baseColor, float(random8(100)) / 100.0 * .1 + .3, 3000, feisty);
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