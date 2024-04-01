#ifndef Animation_h
#define Animation_h

#include <Arduino.h>

#include "system/Graph.h"

class Animation {
 public:
  const char* name = "Base Animation";

  Animation(const char* _name) : name(_name) {}

  virtual byte adjustBrightness(byte brightness) { return brightness; };
  virtual void activate(){};
  virtual void preRender(CRGB leds[TOTAL_LEDS]){};
  virtual void render(CRGB leds[TOTAL_LEDS]){};
  virtual void postRender(CRGB leds[TOTAL_LEDS]){};
};

#endif