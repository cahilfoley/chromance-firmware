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

#if RAINBOW_HORIZONTAL_WAVE_ENABLED || RAINBOW_VERTICAL_WAVE_ENABLED
#include "animation/RainbowWave.h"
#if RAINBOW_HORIZONTAL_WAVE_ENABLED
RainbowWave rainbowHorizontalWave = RainbowWave(horizontalScroll);
#endif
#if RAINBOW_VERTICAL_WAVE_ENABLED
RainbowWave rainbowVerticalWave = RainbowWave(verticalScroll);
#endif
#endif

#if STARBURST_PULSES_ENABLED
#include "animation/StarburstPulses.h"
StarburstPulses starburstPulses;
#endif

#if FLAT_RAINBOW_ENABLED
#include "animation/FlatRainbow.h"
FlatRainbow flatRainbow;
#endif

const byte animationCount = RANDOM_PULSES_ENABLED + CUBE_PULSES_ENABLED + RAINBOW_HORIZONTAL_WAVE_ENABLED +
                            RAINBOW_VERTICAL_WAVE_ENABLED + STARBURST_PULSES_ENABLED + FLAT_RAINBOW_ENABLED;

Animation* animations[animationCount] = {
#if RANDOM_PULSES_ENABLED
    &randomPulses,
#endif
#if RAINBOW_VERTICAL_WAVE_ENABLED
    &rainbowVerticalWave,
#endif
#if CUBE_PULSES_ENABLED
    &cubePulses,
#endif
#if RAINBOW_HORIZONTAL_WAVE_ENABLED
    &rainbowHorizontalWave,
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
  bool canSleep;
  int sleepTimeSeconds;
  byte brightness;
  Animation* animation;
  unsigned long lastAnimationChange;

  StateManager() {
#ifdef ENABLE_TIME_MANAGER
    // If the system is started during the night, start with the night brightness until we confirm it's not night time
    brightness = nightConfig.brightness;
#else
    brightness = dayConfig.brightness;
#endif

    animationIndex = 0;
    canSleep = false;
    sleepTimeSeconds = 0;
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
    // If it's between the night start and end apply the night brightness
    if (time.tm_hour >= nightConfig.start || time.tm_hour <= nightConfig.end) {
      brightness = nightConfig.brightness;

      if (nightConfig.brightness != 0) {
        canSleep = false;
      } else {
        // If we've recently interrupted the sleep cycle with a button press, don't go back to sleep for at least an
        // hour
        bool recentlyWokenByButton = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0 && millis() < 60 * 60 * 1000;

        if (!recentlyWokenByButton) {
          canSleep = true;
          byte hoursToSleep = ((nightConfig.end + 24 - time.tm_hour) % 24) - 1;
          int minutesToSleep = hoursToSleep * 60 + 60 - time.tm_min;
          sleepTimeSeconds = minutesToSleep * 60 + 60 - time.tm_sec;
        }
      }
    }
    // If it's after 6pm start fading the brightness down
    else if (time.tm_hour >= dayConfig.end) {
      brightness = map(time.tm_hour * 60 + time.tm_min, dayConfig.end * 60, nightConfig.start * 60,
                       dayConfig.brightness, nightConfig.brightness);
      canSleep = false;
    }
    // If it's before 8am start fading the brightness up
    else if (time.tm_hour < dayConfig.start) {
      brightness = map(time.tm_hour * 60 + time.tm_min, nightConfig.end * 60, dayConfig.start * 60,
                       nightConfig.brightness, dayConfig.brightness);
      canSleep = false;
    }
    // At other times the lights are on full
    else {
      brightness = dayConfig.brightness;
      canSleep = false;
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
