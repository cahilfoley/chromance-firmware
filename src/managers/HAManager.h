#include "config.h"
#ifdef ENABLE_HOME_ASSISTANT

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
HALight haLight("chromance-light", HALight::BrightnessFeature);
HASelect haSelect("chromance-animation");
HASwitch haSwitch("chromance-cycle");

class HAManager {
 public:
  void setup(const char* mqttBroker, const int mqttPort, const char* mqttUsername, const char* mqttPassword) {
    byte mac[6];
    WiFi.macAddress(mac);
    haDevice.setUniqueId(mac, sizeof(mac));
    haDevice.setName("Chromance");
    haDevice.setModel("ESP32");
    haDevice.setManufacturer("Cahil");
    haDevice.enableSharedAvailability();
    haDevice.enableLastWill();

    haLight.setName("Light");
    haLight.setCurrentState(stateManager.enabled);
    haLight.setCurrentBrightness(stateManager.brightness);
    haLight.onStateCommand(onStateCommand);
    haLight.onBrightnessCommand(onBrightnessCommand);
    stateManager.enabledEmitter.on<bool>(updateState);
    stateManager.brightnessEmitter.on<byte>(updateBrightness);

    haSwitch.setName("Cycle");
    haSwitch.setIcon("mdi:autorenew");
    haSwitch.setCurrentState(stateManager.autoChangeAnimation);
    haSwitch.onCommand(onAutoChangeCommand);
    stateManager.autoChangeEnabledEmitter.on<bool>(updateAutoChange);

    haSelect.setName("Animation");
    haSelect.setOptions(animationOptions);
    haSelect.setState(stateManager.animation->type);
    haSelect.onCommand(onAnimationCommand);
    stateManager.animationEmitter.on<Animation*>(updateAnimation);

    mqttClient.begin(mqttBroker, mqttPort, mqttUsername, mqttPassword);
  };

  void loop() { mqttClient.loop(); };

 private:
  static void onStateCommand(bool state, HALight* sender) {
    Serial.print("Received state command from HA: ");
    Serial.println(state);
    stateManager.setEnabled(state, true);
    sender->setState(state);
  };
  static void updateState(bool state, bool fromHA) {
    if (!fromHA) {
      Serial.print("Updating state from chromance: ");
      Serial.println(state);
      haLight.setState(state);
    } else {
      Serial.print("Not sending state to HA: ");
      Serial.println(state);
    }
  }

  static void onBrightnessCommand(byte brightness, HALight* sender) {
    Serial.print("Received brightness command from HA: ");
    Serial.println(brightness);
    stateManager.setBrightness(brightness, true);
    sender->setBrightness(brightness);
  };
  static void updateBrightness(byte brightness, bool fromHA) {
    if (!fromHA) {
      Serial.print("Updating brightness from chromance: ");
      Serial.println(brightness);
      haLight.setBrightness(brightness);
    } else {
      Serial.print("Not sending brightness to HA: ");
      Serial.println(brightness);
    }
  }

  static void onAutoChangeCommand(bool autoChange, HASwitch* sender) {
    Serial.print("Received auto change command from HA: ");
    Serial.println(autoChange);
    stateManager.setAutoChangeEnabled(autoChange, true);
    sender->setState(autoChange);
  };
  static void updateAutoChange(bool autoChange, bool fromHA) {
    if (!fromHA) {
      Serial.print("Updating auto change from chromance: ");
      Serial.println(autoChange);
      haSwitch.setState(autoChange);
    } else {
      Serial.print("Not sending auto change to HA: ");
      Serial.println(autoChange);
    }
  }

  static void onAnimationCommand(int8_t animation, HASelect* sender) {
    if (animation < 0) return;
    Serial.print("Received animation command from HA: ");
    Serial.println(animation);
    stateManager.setAnimation(animation, true);
  };

  static void updateAnimation(Animation* animation, bool fromHA) {
    if (!fromHA) {
      Serial.print("Updating animation from chromance: ");
      Serial.println(animation->name);
      haSelect.setState(animation->type);
    } else {
      Serial.print("Not sending animation to HA: ");
      Serial.println(animation->name);
    }
  }
};

HAManager haManager;

#endif
#endif