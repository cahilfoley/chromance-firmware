#ifndef FadingAnimation_h
#define FadingAnimation_h

#include "animation/base/Animation.h"

class FadingAnimation : public Animation {
 public:
  FadingAnimation(const char *name, const AnimationType type, byte _fadeRate)
      : Animation(name, type), fadeRate(_fadeRate) {}

  void postRender(CRGB leds[TOTAL_LEDS]) {
    Animation::postRender(leds);
    fadeToBlackBy(leds, TOTAL_LEDS, fadeRate);
  };

 private:
  byte fadeRate;
};

#endif