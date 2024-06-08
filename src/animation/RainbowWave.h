#ifndef RainbowWave_h
#define RainbowWave_h

#include "animation/base/Animation.h"
#include "config.h"

enum WaveDirection { horizontalScroll = 0, verticalScroll = 1, circularWave = 2, pinWheelWave = 3 };
#define NUMBERS_OF_WAVE_DIRECTIONS 4

class RainbowWave : public Animation {
 public:
  WaveDirection direction;
  bool autoChangeDirection;

  RainbowWave() : Animation("Rainbow Wave", RAINBOW_WAVE), direction(horizontalScroll), autoChangeDirection(true) {}
  RainbowWave(WaveDirection direction)
      : Animation("Rainbow Wave", RAINBOW_WAVE), direction(direction), autoChangeDirection(false) {}

  void activate() {
    if (autoChangeDirection) {
      direction = static_cast<WaveDirection>((direction + 1) % NUMBERS_OF_WAVE_DIRECTIONS);
    }

    lastActivationTime = millis();
    auto centerNode = &graph.nodes[starburstNode];
    maxDistanceFromCenter = sqrt(centerNode->x * centerNode->x + centerNode->y * centerNode->y);
  }

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

    int hueShift = map(animationDuration, 0, animationChangeTime, direction == circularWave ? 2500 : 500, 0);

    for (auto &strip : graph.strips) {
      for (auto &led : strip.ledCoordinates) {
        int baseHue = 0;
        if (direction == horizontalScroll) {
          baseHue = map(led.x + hueShift, 0, xLimit, 0, 255);
        } else if (direction == verticalScroll) {
          baseHue = map(led.y + hueShift, 0, yLimit, 0, 255);
        } else if (direction == circularWave) {
          int dx = graph.nodes[starburstNode].x - led.x;
          int dy = graph.nodes[starburstNode].y - led.y;
          int distanceFromCenter = sqrt(dx * dx + dy * dy);
          baseHue = map(distanceFromCenter, 0, maxDistanceFromCenter, 0, 255);
        } else if (direction == pinWheelWave) {
          int dx = graph.nodes[starburstNode].x - led.x;
          int dy = graph.nodes[starburstNode].y - led.y;
          int distanceFromCenter = sqrt(dx * dx + dy * dy);
          baseHue = map(atan2(dy, dx) * 180 / PI + hueShift + distanceFromCenter, 0, 360, 0, 255);
        }

#ifdef FIXED_COLOR_MODE
        float sineWave = sin((baseHue + hueShift) * 3.0 / 255.0);
        int purpleHue = fmap(sineWave, -1, 1, rainbowStartHue, rainbowEndHue);
        leds[led.globalIndex] = CHSV(purpleHue, 255, value);
#else
        leds[led.globalIndex] = CHSV((baseHue + hueShift) % 255, 255, value);
#endif
      }
    }
  };

 private:
  unsigned long lastActivationTime = 0;

  int maxDistanceFromCenter;
};

#endif