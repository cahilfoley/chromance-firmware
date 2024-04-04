#ifndef HAManager_h
#define HAManager_h

#include <ArduinoHA.h>
#include <WiFi.h>

#include "StateManager.h"

WiFiClient client;
HADevice haDevice;
HAMqtt mqttClient(client, haDevice);

// HALight::BrightnessFeature enables support for setting brightness of the light.
// HALight::ColorTemperatureFeature enables support for setting color temperature of the light.
// Both features are optional and you can remove them if they're not needed.
// "chromance" is unique ID of the light. You should define your own ID.
HALight haLight("chromance", HALight::BrightnessFeature);
// HASelect haSelect("chromance");

class HAManager {
 public:
  void setup(const char* mqttBroker, const int mqttPort, const char* mqttUsername, const char* mqttPassword) {
    byte mac[6];
    WiFi.macAddress(mac);
    haDevice.setUniqueId(mac, sizeof(mac));
    haDevice.setName("Chromance");
    haDevice.setModel("ESP32");

    haLight.setName("Chromance");
    haLight.onStateCommand(onStateCommand);
    haLight.onBrightnessCommand(onBrightnessCommand);
    haLight.setCurrentState(stateManager.enabled);
    haLight.setCurrentBrightness(stateManager.brightness);

    stateManager.enabledEmitter.on<bool>(updateState);
    stateManager.brightnessEmitter.on<byte>(updateBrightness);
    stateManager.autoChangeEnabledEmitter.on<bool>([&](bool autoChangeEnabled) {
      // haSelect.setEnabled(autoChangeEnabled);
    });

    // haSelect.setName("Animation");
    // haSelect.setOptions("Random Pulses;Cube Pulses;Flat Rainbow;Rainbow Wave;Starburst Pulses");
    // haSelect.setState(0);
    // haSelect.onCommand(onAnimationCommand);

    mqttClient.begin(mqttBroker, mqttPort, mqttUsername, mqttPassword);
  };

  void loop() { mqttClient.loop(); };

 private:
  static void onStateCommand(bool state, HALight* sender) {
    stateManager.setEnabled(state);
    sender->setState(stateManager.enabled);
  };
  static void onBrightnessCommand(byte brightness, HALight* sender) {
    stateManager.setBrightness(brightness);
    sender->setBrightness(stateManager.brightness);
  };
  // static void onAnimationCommand(int8_t animation, HASelect* sender) {
  //   if (animation < 0) return;
  //   const char* animations[] = {"Random Pulses", "Cube Pulses", "Flat Rainbow", "Rainbow Wave", "Starburst Pulses"};
  //   stateManager.setAnimation(animations[animation]);
  // };

  static void updateState(bool state) { haLight.setState(state); }
  static void updateBrightness(byte brightness) { haLight.setBrightness(brightness); }
  // static void updateAnimation(char* animation) {
  //   int8_t animationIndex = -1;
  //   if (strcmp(animation, "Random Pulses") == 0) {
  //     animationIndex = 0;
  //   } else if (strcmp(animation, "Cube Pulses") == 0) {
  //     animationIndex = 1;
  //   } else if (strcmp(animation, "Flat Rainbow") == 0) {
  //     animationIndex = 2;
  //   } else if (strcmp(animation, "Rainbow Wave") == 0) {
  //     animationIndex = 3;
  //   } else if (strcmp(animation, "Starburst Pulses") == 0) {
  //     animationIndex = 4;
  //   }
  //   haSelect.setState(animationIndex);
  // }
};

HAManager haManager;

#endif