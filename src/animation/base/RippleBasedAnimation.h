#ifndef RippleBasedAnimation_h
#define RippleBasedAnimation_h

#include "animation/base/Animation.h"
#include "animation/base/Ripple.h"

class RippleBasedAnimation : public Animation {
 public:
  RippleBasedAnimation(const char *name) : Animation(name) {}

  void preRender(CRGB leds[TOTAL_LEDS]) {
    Animation::render(leds);
    for (byte i = 0; i < numberOfRipples; i++) {
      if (ripples[i].state != dead) {
        ripples[i].advance(leds);
      }
    }
  };

  void postRender(CRGB leds[TOTAL_LEDS]) {
    Animation::postRender(leds);
    fadeToBlackBy(leds, TOTAL_LEDS, 2);
  };
};

#endif