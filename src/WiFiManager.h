#include <WiFi.h>

#include "secrets.h"

class WiFiManager {
 public:
  void setup() {
    Serial.begin(115200);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID, wifiPassword);

    Serial.println("Connecting to wifi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("Wifi successfully connected with IP address: ");
    Serial.println(WiFi.localIP());
  }
};
