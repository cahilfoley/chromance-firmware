#ifndef MQTTManager_h
#define MQTTManager_h

#include <ArduinoMqttClient.h>
#include <WiFi.h>

#include "secrets.h"

WiFiClient wifiClient;

class MQTTManager {
 public:
  MqttClient client = MqttClient(wifiClient);

  bool setup() {
    client.setId("chromance");
    client.setUsernamePassword(mqttUsername, mqttPassword);

    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(mqttBroker);

    bool connected = client.connect(mqttBroker, mqttPort);

    if (connected) {
      Serial.print("MQTT connection was successful");
    } else {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(client.connectError());
    }

    return connected;
  }

  void poll() { client.poll(); }
};

MQTTManager mqttManager;

#endif