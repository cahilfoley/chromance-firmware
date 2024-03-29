#ifndef StarburstPulses_h
#define StarburstPulses_h

#include "animation/base/RippleBasedAnimation.h"
#include "config.h"

class StarburstPulses : public RippleBasedAnimation {
 public:
  void preRender(CRGB leds[TOTAL_LEDS]) {
    RippleBasedAnimation::preRender(leds);

    unsigned long now = millis();
    if (now - lastRandomPulse <= randomPulseInterval) return;

    lastRandomPulse = now;

    // RippleBehavior behavior = random8(2) ? alwaysTurnsLeft : alwaysTurnsRight;
    RippleBehavior behavior = alwaysTurnsRight;

    lastAutoPulseNode = &graph.nodes[starburstNode];

    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < numberOfRipples; j++) {
        if (ripples[j].state == dead) {
          ripples[j].start(lastAutoPulseNode, i, CHSV(((255 / 6) * (lastColorIndex + i)) % 255, 255, 255), .4, 1800,
                           behavior);

          break;
        }
      }
    }

    lastColorIndex = (lastColorIndex + 1) % colorCount;
  };

 private:
  Node *lastAutoPulseNode;
  byte lastColorIndex = 0;
  unsigned long lastRandomPulse = 0;
};

#endif