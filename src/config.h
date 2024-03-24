#ifndef config_h
#define config_h

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

// The length of each channel in LEDs
const int lengths[CHANNEL_COUNT] = {154, 168, 84, 154};

// Animation configuration //
/** The interval between automatic animation changes */
const int animationChangeTime = 30000;

const bool randomPulsesEnabled = true;  // Fire random pulses from random nodes
const bool cubePulsesEnabled = true;    // Draw cubes at random nodes
const bool starburstPulsesEnabled = true;  // Draw starbursts

// NTP configuration //
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 60 * 60;  // GMT+8
const int daylightOffset_sec = 0;

#endif