#ifndef StarTwinkle_h
#define StarTwinkle_h

#include "animation/base/Animation.h"
#include "config.h"
#include "helpers.h"
#include "system/Graph.h"

const byte numberOfStars = STRIP_COUNT;

enum StarState { starDead, starGrowing, starFading };

struct Star {
  byte index;
  float progress;
  byte targetBrightness;
  int delay;
  StarState state;
  Strip *strip;
};

class StarTwinkle : public Animation {
 public:
  StarTwinkle() : Animation("Star Twinkle", STAR_TWINKLE) {}

  void activate() {
    Animation::activate();

    for (byte i = 0; i < numberOfStars; i++) {
      uint16_t index = random16(TOTAL_LEDS);
      byte targetBrightness = random16(100, 255);
      byte stripIndex = i % STRIP_COUNT;
      byte pixelIndex = random8(3, STRIP_LED_COUNT - 3);
      int delay = random16(10000);
      stars[i] = {pixelIndex, 0, targetBrightness, delay, starGrowing, &graph.strips[stripIndex]};
    }

    lastRender = millis();
  }

  void preRender(CRGB leds[TOTAL_LEDS]) {
    Animation::preRender(leds);
    unsigned long now = millis();
    unsigned long timeSinceLastRender = now - lastRender;

    for (byte i = 0; i < numberOfStars; i++) {
      switch (stars[i].state) {
        case starGrowing:
          if (stars[i].delay > 0) {
            stars[i].delay -= timeSinceLastRender;
          } else if (stars[i].progress >= 1) {
            stars[i].state = starFading;
            stars[i].delay = random16(1000);
          } else {
            stars[i].progress = min(stars[i].progress + timeSinceLastRender * 0.00025, 1.0);
          }
          break;

        case starFading:
          if (stars[i].delay > 0) {
            stars[i].delay -= timeSinceLastRender;
          } else if (stars[i].progress <= 0) {
            stars[i].state = starDead;
          } else {
            stars[i].progress = max(stars[i].progress - timeSinceLastRender * 0.00025, 0.0);
          }
          break;

        case starDead:
          stars[i].state = starGrowing;
          stars[i].delay = random16(7500);
          stars[i].index = random8(3, STRIP_LED_COUNT - 3);
          break;
      }
    }
  }

  void render(CRGB leds[TOTAL_LEDS]) {
    Animation::render(leds);
    unsigned long now = millis();

    for (auto &star : stars) {
      byte value = round(fmap(fmin(fmax(star.progress, 0.0), 1.0), 0.0, 1.0, 0, star.targetBrightness));
      leds[star.strip->ledCoordinates[star.index].globalIndex].setRGB(value, value, value);
    }

    lastRender = now;
  };

 private:
  Star stars[numberOfStars];
  unsigned long lastRender;
};

#endif