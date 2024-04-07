#ifndef RippleBasedAnimation_h
#define RippleBasedAnimation_h

#include "animation/base/FadingAnimation.h"
#include "animation/base/Ripple.h"

class RippleBasedAnimation : public FadingAnimation {
 public:
  RippleBasedAnimation(const char *name, const AnimationType type) : FadingAnimation(name, type, 2) {}

  void preRender(CRGB leds[TOTAL_LEDS]) {
    FadingAnimation::preRender(leds);
    for (byte i = 0; i < numberOfRipples; i++) {
      if (ripples[i].state != dead) {
        ripples[i].advance(leds);
      }
    }
  };
};

#endif