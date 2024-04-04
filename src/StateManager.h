#ifndef StateManager_h
#define StateManager_h

#include <Arduino.h>
#include <EventEmitter.h>

#include <mutex>

#include "config.h"

#ifdef ENABLE_TIME_MANAGER
#include <TimeManager.h>

#include "time.h"
#endif

#include "animation/CubePulses.h"
#include "animation/FlatRainbow.h"
#include "animation/RainbowWave.h"
#include "animation/RandomPulses.h"
#include "animation/StarburstPulses.h"
#include "animation/base/Animation.h"

RandomPulses randomPulses;
CubePulses cubePulses;
RainbowWave rainbowWave;
StarburstPulses starburstPulses;
FlatRainbow flatRainbow;

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
  EventEmitter brightnessEmitter;
  EventEmitter enabledEmitter;
  EventEmitter autoChangeEnabledEmitter;
  EventEmitter animationEmitter;

  bool canSleep = false;
  int sleepTimeSeconds;

  bool enabled;
  bool autoChangeAnimation;
  byte brightness;
  Animation* animation;

  unsigned long lastAnimationChange;

  StateManager() {
#ifdef ENABLE_TIME_MANAGER
    // If the system is started during the night, start with the night brightness until we confirm it's not night time
    setBrightness(nightConfig.brightness);
#else
    setBrightness(255);
#endif

    sleepTimeSeconds = 0;

    autoChangeAnimation = true;
    enabled = true;
    animationIndex = 0;
    animation = animations[animationIndex];
    lastAnimationChange = millis();
    animation->activate();
  }

  void lock() { stateLock.lock(); };
  void unlock() { stateLock.unlock(); };

  void setEnabled(bool newEnabled) {
    lock();
    enabled = newEnabled;
    unlock();
    enabledEmitter.emit(enabled);
  };

  void setBrightness(byte newBrightness) {
    lock();
    brightness = newBrightness;
    unlock();
    brightnessEmitter.emit(brightness);
  };

  void selectNextAnimation() {
    if (!autoChangeAnimation) return;

    lock();
    animationIndex = (animationIndex + 1) % ANIMATION_COUNT;
    animation = animations[animationIndex];
    animation->activate();
    lastAnimationChange = millis();
    unlock();

    animationEmitter.emit(animation);
  };

  void setAnimation(const char* animationName) {
    lock();
    for (int i = 0; i < ANIMATION_COUNT; i++) {
      if (strcmp(animationName, animations[i]->name) == 0) {
        animationIndex = i;
        animation = animations[animationIndex];
        animation->activate();
        lastAnimationChange = millis();
        break;
      }
    }
    unlock();
    animationEmitter.emit(animation);
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
