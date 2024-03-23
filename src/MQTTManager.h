#ifndef MQTTManager_h
#define MQTTManager_h

#include <ArduinoMqttClient.h>
#include <WiFi.h>

#include "secrets.h"

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

class MQTTManager {
 public:
  bool setup() {
    mqttClient.setId("chromance");
    mqttClient.setUsernamePassword(mqttUsername, mqttPassword);

    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(mqttBroker);

    bool connected = mqttClient.connect(mqttBroker, mqttPort);

    if (connected) {
      Serial.print("MQTT connection was successful");
    } else {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
    }

    return connected;
  }

  void poll() { mqttClient.poll(); }
};

#endif