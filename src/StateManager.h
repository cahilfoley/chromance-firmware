#ifndef StateManager_h
#define StateManager_h

#include <Arduino.h>
#include <EventManager.h>

#include <mutex>

#include "config.h"
#include "time.h"

// Auto pulse configuration
#define NUM_AUTO_PULSE_TYPES 4
enum AnimationType {
  RandomPulses = 0,
  CubePulses = 1,
  StarburstPulses = 2,
  FlatRainbow = 3
};
const char* animationNames[] = {
    "Random Pulses",
    "Cube Pulses",
    "Starburst Pulses",
    "System Pulse",
};

#define randomPulseTime 2000  // Fire a random pulse every (this many) ms

class StateManager : public EventManager {
 public:
  byte brightness;
  AnimationType animation;
  unsigned long lastAnimationChange;
  std::mutex stateLock;

  StateManager() {
    brightness = 255;
    animation = FlatRainbow;
    lastAnimationChange = millis();
  }

  void selectNextAnimation() {
    stateLock.lock();
    findNextAnimation();
    lastAnimationChange = millis();
    stateLock.unlock();
    emit("animationChange", animation);
  }

  void updateBrightnessFromTime(struct tm time) {
    stateLock.lock();
    // If it's between 10pm and 6am, turn off the lights
    if (time.tm_hour >= 22 || time.tm_hour <= 6) {
      brightness = 0;
    }
    // If it's after 6pm start fading the brightness down
    else if (time.tm_hour >= 18) {
      brightness =
          map(time.tm_hour * 60 + time.tm_min, 18 * 60, 22 * 60, 255, 20);
    }
    // If it's before 8am start fading the brightness up
    else if (time.tm_hour < 8) {
      brightness =
          map(time.tm_hour * 60 + time.tm_min, 6 * 60, 8 * 60, 20, 255);
    }
    // At other times the lights are on full
    else {
      brightness = 255;
    }

    stateLock.unlock();
    emit("brightnessChange", brightness);
  }

 private:
  void findNextAnimation() {
    animation = (AnimationType)((animation + 1) % NUM_AUTO_PULSE_TYPES);
    switch (animation) {
      case RandomPulses:
        if (!randomPulsesEnabled) return findNextAnimation();
      case CubePulses:
        if (!cubePulsesEnabled) return findNextAnimation();
      case StarburstPulses:
        if (!starburstPulsesEnabled) return findNextAnimation();
      case FlatRainbow:
        if (!flatRainbowEnabled) return findNextAnimation();
    }
  }
};

StateManager stateManager;

#endif
