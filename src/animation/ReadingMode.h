#ifndef ReadingMode_h
#define ReadingMode_h

#include "animation/base/Animation.h"
#include "config.h"

class ReadingMode : public Animation {
 private:
  bool firstRender = true;  // Flag to track if this is the first render

 public:
  ReadingMode() : Animation("Reading Mode", READING_MODE) {
    // This animation should not be included in automatic cycling
    includeInAutoCycle = false;
  }
  void activate() {
    // No specific activation logic needed for reading mode
    firstRender = true;  // Reset the first render flag
  };

  bool render(CRGB leds[TOTAL_LEDS]) {
    Animation::render(leds);  // Call parent render

    // If this is the first render, set all LEDs to a warm white color
    if (firstRender) {
      // True warm white color - reducing blue component and adding more red
      // These values create a warm, slightly yellow-orange tint (2700K-3000K color temperature)
      const CRGB warmWhite = CRGB(255, 223, 175);  // Warm white with reduced blue component

      // Set all LEDs to warm white
      for (int i = 0; i < TOTAL_LEDS; i++) {
        leds[i] = warmWhite;
      }

      firstRender = false;
      return true;  // LEDs need to be updated
    }

    return false;  // No update needed after the first render
  };
};

#endif
