#ifndef StateManager_h
#define StateManager_h

#include <Arduino.h>
#include <TimeManager.h>

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

#if RAINBOW_WAVE_ENABLED
#include "animation/RainbowWave.h"
RainbowWave rainbowWave;
#endif

#if STARBURST_PULSES_ENABLED
#include "animation/StarburstPulses.h"
StarburstPulses starburstPulses;
#endif

#if FLAT_RAINBOW_ENABLED
#include "animation/FlatRainbow.h"
FlatRainbow flatRainbow;
#endif

Animation* animations[] = {
#if RANDOM_PULSES_ENABLED
    &randomPulses,
#endif
#if RAINBOW_WAVE_ENABLED
    &rainbowWave,
#endif
#if CUBE_PULSES_ENABLED
    &cubePulses,
#endif
#if RAINBOW_WAVE_ENABLED
    &rainbowWave,
#endif
#if STARBURST_PULSES_ENABLED
    &starburstPulses,
#endif
#if RAINBOW_WAVE_ENABLED
    &rainbowWave,
#endif
#if FLAT_RAINBOW_ENABLED
    &flatRainbow,
#endif
};

const int ANIMATION_COUNT = sizeof(animations) / sizeof(animations[0]);

class StateManager {
 public:
  bool canSleep;
  bool autoBrightness = true;
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
    animation->activate();
  }

  void lock() { stateLock.lock(); };
  void unlock() { stateLock.unlock(); };

  void toggleAutoBrightness() {
    lock();
    autoBrightness = !autoBrightness;
#ifdef ENABLE_TIME_MANAGER
    if (autoBrightness) {
      // If we're toggling auto brightness on, update the brightness based on the current time
      auto time = timeManager.getCurrentLocalTime();
      updateBrightnessFromTime(time);
    } else {
      // If we're toggling auto brightness off, set the brightness to full
      brightness = 255;
    }
#endif
    unlock();
  };

  void selectNextAnimation() {
    lock();
    animationIndex = (animationIndex + 1) % ANIMATION_COUNT;
    animation = animations[animationIndex];
    animation->activate();
    lastAnimationChange = millis();
    unlock();
  };

#ifdef ENABLE_TIME_MANAGER
  void updateBrightnessFromTime(struct tm time) {
    if (!autoBrightness) return;

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
          byte hoursToSleep = (nightConfig.end + 24 - time.tm_hour) % 24;
          if (hoursToSleep > 0) hoursToSleep -= 1;  // Only subtract 1 if hoursToSleep is not 0
          int minutesToSleep = hoursToSleep * 60 + 59 - time.tm_min;
          sleepTimeSeconds = minutesToSleep * 60 + 59 - time.tm_sec;
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
