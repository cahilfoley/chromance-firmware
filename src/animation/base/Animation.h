#ifndef Animation_h
#define Animation_h

#include <Arduino.h>

#include "system/Graph.h"

class Animation {
 public:
  Animation() {}

  virtual byte adjustBrightness(byte brightness) { return brightness; };
  virtual void activate(){};
  virtual void preRender(CRGB leds[TOTAL_LEDS]){};
  virtual void render(CRGB leds[TOTAL_LEDS]){};
  virtual void postRender(CRGB leds[TOTAL_LEDS]){};
};

#endif