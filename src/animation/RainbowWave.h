#ifndef RainbowWave_h
#define RainbowWave_h

#include "animation/base/Animation.h"
#include "config.h"

enum Direction { horizontalScroll = 0, verticalScroll = 1 };

class RainbowWave : public Animation {
 public:
  Direction direction;

  RainbowWave() : Animation(), direction(horizontalScroll) {}
  RainbowWave(Direction direction) : Animation(), direction(direction) {}

  void activate() { lastActivationTime = millis(); }

  void render(CRGB leds[TOTAL_LEDS]) {
    Animation::render(leds);
    unsigned long now = millis();

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

    int hueShift = map(animationDuration, 0, animationChangeTime, 500, 0);

    for (auto &strip : graph.strips) {
      for (auto &led : strip.leds) {
        int baseHue;
        if (direction == horizontalScroll) {
          baseHue = map(led.x + hueShift, 0, xLimit, 0, 255);
        } else {
          baseHue = map(led.y + hueShift, 0, yLimit, 0, 255);
        }

        leds[led.globalIndex] = CHSV((baseHue + hueShift) % 255, 255, value);
      }
    }
  };

 private:
  unsigned long lastActivationTime = 0;
};

#endif