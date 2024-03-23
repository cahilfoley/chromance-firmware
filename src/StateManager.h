#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>

#include "config.h"
#include "lib/EventManager.h"

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
  uint8_t brightness;
  AutoPulseType animation;
  int lastAnimationChange;

  StateManager() {
    brightness = 255;
    animation = RandomPulses;
    lastAnimationChange = millis();
  }

  void selectNextAnimation() {
    animation = (AutoPulseType)((animation + 1) % numberOfAutoPulseTypes);
    lastAnimationChange = millis();
    emit("animationChange", animation);
  }
};

StateManager stateManager;

#endif