#ifndef FlatRainbow_h
#define FlatRainbow_h

#include "animation/base/Animation.h"
#include "config.h"

class FlatRainbow : public Animation {
 public:
  void activate() { lastActivationTime = millis(); }

  void render(CRGB leds[TOTAL_LEDS]) {
    Animation::render(leds);
    unsigned long now = millis();

    byte hue = map(now, lastActivationTime, lastActivationTime + animationChangeTime, 0, 255);

    // Fade the brightness in and out at the start and end of the animation
    byte value = 200;
    int animationDuration = (now - lastActivationTime);
    if (animationDuration < 1000) {
      value = map(animationDuration, 0, 1000, 0, 200);
    } else if (animationDuration > animationChangeTime) {
      value = 0;
    } else if (animationDuration > animationChangeTime - 1000) {
      value = map(animationDuration, animationChangeTime - 1000, animationChangeTime + 1, 200, 0);
    }

    for (int i = 0; i < TOTAL_LEDS; i++) {
      leds[i] = CHSV(hue, 255, value);
    }
  };

 private:
  unsigned long lastActivationTime = 0;
};

#endif