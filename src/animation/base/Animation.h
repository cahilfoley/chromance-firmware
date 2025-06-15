#ifndef Animation_h
#define Animation_h

#include <Arduino.h>

#include "constants.h"
#include "system/Graph.h"

class Animation {
 public:
  const char* name = "Base Animation";
  AnimationType type;
  bool includeInAutoCycle = true;  // By default, animations are included in automatic cycling

  Animation(const char* _name, const AnimationType _type) : name(_name), type(_type) {}

  virtual byte adjustBrightness(byte brightness) { return brightness; };
  virtual void activate() {};
  virtual void preRender(CRGB leds[TOTAL_LEDS]) {};

  // Render method that return a boolean indicating whether LEDs need to be updated
  // Default is true (LEDs need updating), subclasses can override to return false when no update is needed
  virtual bool render(CRGB leds[TOTAL_LEDS]) { return true; };

  virtual void postRender(CRGB leds[TOTAL_LEDS]) {};
};

#endif