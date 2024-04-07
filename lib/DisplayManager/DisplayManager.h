#ifndef DisplayManager_h
#define DisplayManager_h

#include <U8g2lib.h>

#include "StateManager.h"
#include "WifiManager.h"
#include "config.h"

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R2, 22, 21, U8X8_PIN_NONE);

class DisplayManager {
 public:
  void setup() {
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_scrum_tr);
    updateDisplay();

    stateManager.brightnessEmitter.on<byte>([&](byte brightness, bool fromHA) { updateDisplay(); });
    stateManager.enabledEmitter.on<bool>([&](bool enabled, bool fromHA) { updateDisplay(); });
    stateManager.autoChangeEnabledEmitter.on<bool>([&](bool enabled, bool fromHA) { updateDisplay(); });
    stateManager.animationEmitter.on<Animation *>([&](Animation *animation, bool fromHA) { updateDisplay(); });
  };

  void showMessage(const char *message) {
    u8g2.clearBuffer();
    u8g2.drawStr(5, 10, message);
    u8g2.sendBuffer();
  };

  void updateDisplay() {
#ifdef ENABLE_SCREEN
    u8g2.clearBuffer();
    char animationBuffer[50];
    sprintf(animationBuffer, "A: %s", stateManager.animation->name);
    u8g2.drawStr(5, 10, animationBuffer);
    char brightnessBuffer[20];
    sprintf(brightnessBuffer, "B: %d", stateManager.brightness);
    u8g2.drawStr(5, 25, brightnessBuffer);
    char autoPlayBuffer[20];
    sprintf(autoPlayBuffer, "AC: %s", stateManager.autoChangeAnimation ? "On" : "Off");
    u8g2.drawStr(60, 25, autoPlayBuffer);
#if defined(ENABLE_TIME_MANAGER) || defined(ENABLE_OTA) || defined(ENABLE_HOME_ASSISTANT)
    char wifiBuffer[50];
    sprintf(wifiBuffer, "W: %s", wifiManager.isConnected() ? "Connected" : "Disconnected");
    u8g2.drawStr(5, 40, wifiBuffer);
#endif
    u8g2.sendBuffer();
#endif
  };
};

DisplayManager displayManager;

#endif