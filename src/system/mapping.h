#ifndef mapping_h
#define mapping_h

#include "config.h"

#define headof(S) ((S - 1) * 14)
#define tailof(S) (headof(S) + 13)

// The length of each channel in LEDs
const int lengths[CHANNEL_COUNT] = {154, 168, 84, 154};

int channelOffsets[CHANNEL_COUNT] = {
    0,
    lengths[0],
    lengths[0] + lengths[1],
    lengths[0] + lengths[1] + lengths[2],
};

// Beam 0 is at 12:00 and advance clockwise
// -1 means nothing connected on that side

struct NodeConfig {
  int stripsIndexes[6];
  bool borderNode;
  byte row;
  byte column;
};
NodeConfig nodeConnections[25] = {{{-1, -1, 1, -1, 0, -1}, true, 0, 5},    {{-1, -1, 3, -1, 2, -1}, true, 0, 3},
                                  {{-1, -1, 5, -1, 4, -1}, true, 0, 1},

                                  {{-1, 0, 6, 12, -1, -1}, true, 1, 6},    {{-1, 2, 8, 14, 7, 1}, false, 1, 4},
                                  {{-1, 4, 10, 16, 9, 3}, false, 1, 2},    {{-1, -1, -1, 18, 11, 5}, true, 1, 0},

                                  {{-1, 7, -1, 13, -1, 6}, false, 2, 5},   {{-1, 9, -1, 15, -1, 8}, false, 2, 3},
                                  {{-1, 11, -1, 17, -1, 10}, false, 2, 1},

                                  {{12, -1, 19, -1, -1, -1}, true, 3, 6},  {{14, -1, 21, -1, 20, -1}, false, 3, 4},
                                  {{16, -1, 23, -1, 22, -1}, false, 3, 2}, {{18, -1, -1, -1, 24, -1}, true, 3, 0},

                                  {{13, 20, 25, 29, -1, 19}, true, 4, 5},  {{15, 22, 27, 31, 26, 21}, false, 4, 3},
                                  {{17, 24, -1, 33, 28, 23}, true, 4, 1},

                                  {{-1, 26, -1, 30, -1, 25}, false, 5, 4}, {{-1, 28, -1, 32, -1, 27}, false, 5, 2},

                                  {{29, -1, 34, -1, -1, -1}, true, 6, 5},  {{31, -1, 36, -1, 35, -1}, false, 6, 3},
                                  {{33, -1, -1, -1, 37, -1}, true, 6, 1},

                                  {{30, 35, 38, -1, -1, 34}, true, 7, 4},  {{32, 37, -1, -1, 39, 36}, true, 7, 2},

                                  {{-1, 39, -1, -1, -1, 38}, true, 8, 3}};

const int xLimit = 7 * (STRIP_LED_COUNT + 1);
const int yLimit = 9 * (STRIP_LED_COUNT + 1);

enum StripOrientation {
  downLeft = 0,
  vertical = 1,
  downRight = 2,
};

struct StripConfig {
  byte channelIndex;
  byte topNodeIndex;
  byte bottomNodeIndex;
  uint16_t topLEDIndex;
  uint16_t bottomLEDIndex;
  byte row;
  byte column;
  StripOrientation orientation;
};

// First member: Strip number
// Second: LED index closer to ceiling
// Third: LED index closer to floor
StripConfig stripConfigs[40] = {
    {2, 0, 3, headof(3), tailof(3), 0, 5, downRight},   {2, 0, 4, tailof(2), headof(2), 0, 4, downLeft},
    {1, 1, 4, headof(10), tailof(10), 0, 3, downRight}, {1, 1, 5, tailof(9), headof(9), 0, 2, downLeft},
    {1, 2, 5, headof(4), tailof(4), 0, 1, downRight},   {1, 2, 6, tailof(3), headof(3), 0, 0, downLeft},

    {2, 3, 7, tailof(6), headof(6), 1, 5, downLeft},    {3, 4, 7, tailof(11), headof(11), 1, 4, downRight},
    {1, 4, 8, headof(11), tailof(11), 1, 3, downLeft},  {1, 5, 8, tailof(8), headof(8), 1, 2, downRight},
    {1, 5, 9, headof(12), tailof(12), 1, 1, downLeft},  {0, 6, 9, tailof(11), headof(11), 1, 0, downRight},

    {2, 3, 10, headof(4), tailof(4), 2, 6, vertical},   {3, 7, 14, tailof(10), headof(10), 2, 5, vertical},
    {2, 4, 11, tailof(1), headof(1), 2, 4, vertical},   {1, 8, 15, tailof(7), headof(7), 2, 3, vertical},
    {1, 5, 12, headof(5), tailof(5), 2, 2, vertical},   {0, 9, 16, tailof(10), headof(10), 2, 1, vertical},
    {1, 6, 13, tailof(2), headof(2), 2, 0, vertical},

    {2, 10, 14, headof(5), tailof(5), 3, 5, downLeft},  {3, 11, 14, tailof(4), headof(4), 3, 4, downRight},
    {3, 11, 15, headof(5), tailof(5), 3, 3, downLeft},  {0, 12, 15, headof(5), tailof(5), 3, 2, downRight},
    {0, 12, 16, tailof(4), headof(4), 3, 1, downLeft},  {1, 13, 16, tailof(1), headof(1), 3, 0, downRight},

    {3, 14, 17, tailof(9), headof(9), 4, 3, downLeft},  {0, 15, 17, headof(6), tailof(6), 4, 2, downRight},
    {1, 15, 18, tailof(6), headof(6), 4, 1, downLeft},  {0, 16, 18, tailof(9), headof(9), 4, 0, downRight},

    {3, 14, 19, tailof(3), headof(3), 5, 4, vertical},  {3, 17, 22, tailof(8), headof(8), 5, 3, vertical},
    {3, 15, 20, headof(6), tailof(6), 5, 2, vertical},  {0, 18, 23, tailof(8), headof(8), 5, 1, vertical},
    {0, 16, 21, tailof(3), headof(3), 5, 0, vertical},

    {3, 19, 22, tailof(2), headof(2), 6, 3, downLeft},  {3, 20, 22, headof(7), tailof(7), 6, 2, downRight},
    {0, 20, 23, headof(7), tailof(7), 6, 1, downLeft},  {0, 21, 23, tailof(2), headof(2), 6, 0, downRight},

    {3, 22, 24, tailof(1), headof(1), 7, 1, downLeft},  {0, 23, 24, tailof(1), headof(1), 7, 0, downRight},
};

const byte stripRowCount = 8;
const byte stripColumnCount = 7;

// Cube nodes link three equiangular segments
// Firing ripples that always turn in one direction will draw a cube
int numberOfCubeNodes = 8;
int cubeNodes[] = {7, 8, 9, 11, 12, 17, 18};

// Firing ripples that always turn in one direction will draw a starburst
int starburstNode = 15;

#endif
