#ifndef config_h
#define config_h

#define ENABLE_LEDS
#define ENABLE_TIME_MANAGER  // Connects to NTP server to get time and adjusts the brightness at night
#define ENABLE_OTA
#define ENABLE_HOME_ASSISTANT
// #define ENABLE_SCREEN
// #define ENABLE_BENCHMARK
// #define ENABLE_BENCHMARK_BACKGROUND
// #define WAIT_FOR_SERIAL
// #define FIXED_COLOR_MODE

#include <FastLED.h>

// The number of channels in the system
const int CHANNEL_COUNT = 4;
// The number of strips in the system
const int STRIP_COUNT = 40;
// The number of LEDs in each strip
const int STRIP_LED_COUNT = 14;
// The total number of LEDs in the system
const int TOTAL_LEDS = STRIP_COUNT * STRIP_LED_COUNT;
// The number of nodes in the system
const int NODE_COUNT = 25;

// Animation configuration //
/** The interval between automatic animation changes */
const int animationChangeTime = 30000;

#define RANDOM_PULSES_ENABLED true     // Fire random pulses from random nodes
#define CUBE_PULSES_ENABLED true       // Draw cubes at random nodes
#define RAINBOW_WAVE_ENABLED true      // Draw a rainbow across the system
#define STARBURST_PULSES_ENABLED true  // Draw starbursts
#define FLAT_RAINBOW_ENABLED false     // Draw a flat colour that cycles through the rainbow
#define STAR_TWINKLE_ENABLED true      // Draw twinkling stars
#define READING_MODE_ENABLED true      // Set all LEDs to warm white for reading

/** For random pulse animations, the duration between pulses */
const int randomPulseInterval = 2000;

// Color configuration //
const byte colorCount = 6;
#ifdef FIXED_COLOR_MODE
const CRGB colors[colorCount] = {
    CRGB(200, 0, 255), CRGB(255, 0, 200), CRGB(220, 0, 255),
    CRGB(255, 0, 220), CRGB(220, 0, 220), CRGB(255, 0, 255)  // Magenta
};
const byte rainbowStartHue = 210;
const byte rainbowEndHue = 240;
const byte starRGB[3] = {255, 0, 255};
#else
const CRGB colors[colorCount] = {
    CRGB(255, 0, 0),    // Red
    CRGB(255, 255, 0),  // Yellow
    CRGB(0, 255, 0),    // Green
    CRGB(0, 255, 255),  // Cyan
    CRGB(0, 0, 255),    // Blue
    CRGB(255, 0, 255)   // Magenta
};
const byte rainbowStartHue = 0;
const byte rainbowEndHue = 255;
const byte starRGB[3] = {255, 255, 255};
#endif

// NTP configuration //
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 60 * 60;  // GMT+8
const int daylightOffset_sec = 0;

/**
 * The brightness levels for day and night will be in effect between the day hours and night hours below.
 *
 * If there is a gap between the day hours and the night hours, the brightness will ramp up or down over that period.
 */
struct CycleConfig {
  byte brightness;
  byte start;
  byte end;
};

const CycleConfig dayConfig = {255, 10, 18};  // 10am to 6pm
const CycleConfig nightConfig = {50, 21, 7};  // 9pm to 7am

#endif