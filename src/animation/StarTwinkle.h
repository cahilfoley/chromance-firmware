#ifndef StarTwinkle_h
#define StarTwinkle_h

#include "animation/base/FadingAnimation.h"
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

// The distance from the end of each strip that a star cannot be placed to prevent clashing with the next strip
#define STAR_STRIP_END_BUFFER 1

// The rates at which stars will grow and fade
#define STAR_GROWTH_RATE 0.00025
#define STAR_DECAY_RATE 0.00025

// The max delay between a star fading out and being reactivated
#define STAR_REACTIVATION_MAX_DELAY 10000

// The max delay between a star reaching full brightness and starting to fade out
#define STAR_DECAY_MAX_DELAY 100

// The min and max range that a star's target brightness will be generated between
#define STAR_MIN_TARGET_BRIGHTNESS 50
#define STAR_MAX_TARGET_BRIGHTNESS 200

class StarTwinkle : public FadingAnimation {
 public:
  StarTwinkle() : FadingAnimation("Star Twinkle", STAR_TWINKLE, 2) {}

  void activate() {
    FadingAnimation::activate();

    for (byte i = 0; i < numberOfStars; i++) {
      uint16_t index = random16(TOTAL_LEDS);
      byte targetBrightness = random16(STAR_MIN_TARGET_BRIGHTNESS, STAR_MAX_TARGET_BRIGHTNESS);
      byte stripIndex = i % STRIP_COUNT;
      byte pixelIndex = random8(STAR_STRIP_END_BUFFER, STRIP_LED_COUNT - STAR_STRIP_END_BUFFER);
      int delay = random16(STAR_REACTIVATION_MAX_DELAY);
      stars[i] = {pixelIndex, 0, targetBrightness, delay, starGrowing, &graph.strips[stripIndex]};
    }

    lastRender = millis();
  }

  void preRender(CRGB leds[TOTAL_LEDS]) {
    FadingAnimation::preRender(leds);
    unsigned long now = millis();
    unsigned long timeSinceLastRender = now - lastRender;

    for (byte i = 0; i < numberOfStars; i++) {
      switch (stars[i].state) {
        case starGrowing:
          if (stars[i].delay > 0) {
            stars[i].delay -= timeSinceLastRender;
          } else if (stars[i].progress >= 1) {
            stars[i].state = starFading;
            stars[i].delay = random16(STAR_DECAY_MAX_DELAY);
          } else {
            stars[i].progress = min(stars[i].progress + timeSinceLastRender * STAR_GROWTH_RATE, 1.0);
          }
          break;

        case starFading:
          if (stars[i].delay > 0) {
            stars[i].delay -= timeSinceLastRender;
          } else if (stars[i].progress <= 0) {
            stars[i].state = starDead;
          } else {
            stars[i].progress = max(stars[i].progress - timeSinceLastRender * STAR_DECAY_RATE, 0.0);
          }
          break;

        case starDead:
          stars[i].state = starGrowing;
          stars[i].delay = random16(STAR_REACTIVATION_MAX_DELAY);
          stars[i].index = random8(STAR_STRIP_END_BUFFER, STRIP_LED_COUNT - STAR_STRIP_END_BUFFER);
          break;
      }
    }
  }

  bool render(CRGB leds[TOTAL_LEDS]) {
    FadingAnimation::render(leds);
    unsigned long now = millis();

    for (auto &star : stars) {
      // Set the brightness of the star based on its progress, clamped to the target brightness
      byte value = round(fmap(fmin(fmax(star.progress, 0.0), 1.0), 0.0, 1.0, 0, star.targetBrightness));
      leds[star.strip->ledCoordinates[star.index].globalIndex].setRGB(
          map(value, 0, 255, 0, starRGB[0]), map(value, 0, 255, 0, starRGB[1]), map(value, 0, 255, 0, starRGB[2]));
    }

    lastRender = now;

    return true;  // Always return true to indicate that the LEDs need updating
  };

 private:
  Star stars[numberOfStars];
  unsigned long lastRender;
};

#endif