#ifndef StateManager_h
#define StateManager_h

#include <Arduino.h>

#include <mutex>

#include "config.h"

#ifdef ENABLE_TIME_MANAGER
#include "time.h"
#endif

#include "animation/base/Animation.h"

#if RANDOM_PULSES_ENABLED
#include "animation/RandomPulses.h"
RandomPulses randomPulses;
#endif

#if CUBE_PULSES_ENABLED
#include "animation/CubePulses.h"
CubePulses cubePulses;
#endif

#if STARBURST_PULSES_ENABLED
#include "animation/StarburstPulses.h"
StarburstPulses starburstPulses;
#endif

#if FLAT_RAINBOW_ENABLED
#include "animation/FlatRainbow.h"
FlatRainbow flatRainbow;
#endif

const int animationCount =
    RANDOM_PULSES_ENABLED + CUBE_PULSES_ENABLED + STARBURST_PULSES_ENABLED + FLAT_RAINBOW_ENABLED;

Animation* animations[animationCount] = {
#if RANDOM_PULSES_ENABLED
    &randomPulses,
#endif
#if CUBE_PULSES_ENABLED
    &cubePulses,
#endif
#if STARBURST_PULSES_ENABLED
    &starburstPulses,
#endif
#if FLAT_RAINBOW_ENABLED
    &flatRainbow,
#endif
};

class StateManager {
 public:
  byte brightness;
  Animation* animation;
  unsigned long lastAnimationChange;

  StateManager() {
    brightness = 255;
    animationIndex = 0;
    animation = animations[animationIndex];
    lastAnimationChange = millis();
  }

  void lock() { stateLock.lock(); };
  void unlock() { stateLock.unlock(); };

  void selectNextAnimation() {
    lock();
    animationIndex = (animationIndex + 1) % animationCount;
    animation = animations[animationIndex];
    animation->activate();
    lastAnimationChange = millis();
    unlock();
  };

#ifdef ENABLE_TIME_MANAGER
  void updateBrightnessFromTime(struct tm time) {
    lock();
    // If it's between 10pm and 6am, turn off the lights
    if (time.tm_hour >= 22 || time.tm_hour <= 6) {
      brightness = 0;
    }
    // If it's after 6pm start fading the brightness down
    else if (time.tm_hour >= 18) {
      brightness = map(time.tm_hour * 60 + time.tm_min, 18 * 60, 22 * 60, 255, 20);
    }
    // If it's before 8am start fading the brightness up
    else if (time.tm_hour < 8) {
      brightness = map(time.tm_hour * 60 + time.tm_min, 6 * 60, 8 * 60, 20, 255);
    }
    // At other times the lights are on full
    else {
      brightness = 255;
    }

    unlock();
  };
#endif

 private:
  byte animationIndex = 0;
  std::mutex stateLock;
};

StateManager stateManager;

#endif
