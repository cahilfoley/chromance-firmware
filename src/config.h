#ifndef config_h
#define config_h

// The number of strips in the system
const int STRIP_COUNT = 40;
// The number of LEDs in each strip
const int STRIP_LED_COUNT = 14;

// The length of each channel in LEDs
const int lengths[] = {154, 168, 84, 154};

// Animation configuration //
/** The interval between automatic animation changes */
const int animationChangeTime = 30000;

const bool randomPulsesEnabled = true;  // Fire random pulses from random nodes
const bool cubePulsesEnabled = true;    // Draw cubes at random nodes
const bool starburstPulsesEnabled = true;  // Draw starbursts

#endif