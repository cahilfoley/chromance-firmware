#ifndef WiFiManager_h
#define WiFiManager_h

#include <WiFi.h>

class WiFiManager {
 public:
  void setup(const char *wifiSSID, const char *wifiPassword) {
    Serial.begin(115200);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("chromance-esp32");
    WiFi.begin(wifiSSID, wifiPassword);

    Serial.println("Connecting to wifi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("Wifi successfully connected with IP address: ");
    Serial.println(WiFi.localIP());
  }

  void disconnect() {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
  }

  bool isConnected() { return WiFi.status() == WL_CONNECTED; }
};

WiFiManager wifiManager;

#endif
