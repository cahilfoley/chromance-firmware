#ifndef StateManager_h
#define StateManager_h

#include <Arduino.h>
#include <EventManager.h>

#include "config.h"
#include "time.h"

// Auto pulse configuration
enum AutoPulseType { RandomPulses = 0, CubePulses = 1, StarburstPulses = 2 };
const char* autoPulseNames[] = {
    "Random Pulses",
    "Cube Pulses",
    "Starburst Pulses",
};

#define randomPulseTime 2000  // Fire a random pulse every (this many) ms

byte numberOfAutoPulseTypes =
    randomPulsesEnabled + cubePulsesEnabled + starburstPulsesEnabled;

class StateManager : public EventManager {
 public:
  byte brightness;
  AutoPulseType animation;
  unsigned long lastAnimationChange;

  StateManager() {
    brightness = 255;
    animation = RandomPulses;
    lastAnimationChange = millis();
  }

  void selectNextAnimation() {
    animation = (AutoPulseType)((animation + 1) % numberOfAutoPulseTypes);
    switch (animation) {
      case RandomPulses:
        if (!randomPulsesEnabled) return selectNextAnimation();
      case CubePulses:
        if (!cubePulsesEnabled) return selectNextAnimation();
      case StarburstPulses:
        if (!starburstPulsesEnabled) return selectNextAnimation();
    }
    lastAnimationChange = millis();
    emit("animationChange", animation);
  }

  void updateBrightnessFromTime(struct tm time) {
    // If it's between 10pm and 6am, turn off the lights
    if (time.tm_hour >= 22 || time.tm_hour <= 6) {
      brightness = 0;
    }
    // If it's after 6pm start fading the brightness down
    else if (time.tm_hour >= 18) {
      brightness =
          map(time.tm_hour * 60 + time.tm_min, 18 * 60, 22 * 60, 255, 50);
    }
    // If it's before 8am start fading the brightness up
    else if (time.tm_hour <= 8) {
      brightness =
          map(time.tm_hour * 60 + time.tm_min, 6 * 60, 8 * 60, 50, 255);
    }
    // At other times the lights are on full
    else {
      brightness = 255;
    }

    emit("brightnessChange", brightness);
  }
};

StateManager stateManager;

#endif
