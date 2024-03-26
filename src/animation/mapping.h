#ifndef mapping_h
#define mapping_h

#include "config.h"

#define headof(S) ((S - 1) * 14)
#define tailof(S) (headof(S) + 13)

int channelOffsets[CHANNEL_COUNT] = {
    0,
    lengths[0],
    lengths[0] + lengths[1],
    lengths[0] + lengths[1] + lengths[2],
};

// Beam 0 is at 12:00 and advance clockwise
// -1 means nothing connected on that side
int nodeConnections[25][6] = {
    {-1, -1, 1, -1, 0, -1},   {-1, -1, 3, -1, 2, -1},
    {-1, -1, 5, -1, 4, -1},   {-1, 0, 6, 12, -1, -1},
    {-1, 2, 8, 14, 7, 1},

    {-1, 4, 10, 16, 9, 3},    {-1, -1, -1, 18, 11, 5},
    {-1, 7, -1, 13, -1, 6},   {-1, 9, -1, 15, -1, 8},
    {-1, 11, -1, 17, -1, 10},

    {12, -1, 19, -1, -1, -1}, {14, -1, 21, -1, 20, -1},
    {16, -1, 23, -1, 22, -1}, {18, -1, -1, -1, 24, -1},
    {13, 20, 25, 29, -1, 19},

    {15, 22, 27, 31, 26, 21}, {17, 24, -1, 33, 28, 23},
    {-1, 26, -1, 30, -1, 25}, {-1, 28, -1, 32, -1, 27},
    {29, -1, 34, -1, -1, -1},

    {31, -1, 36, -1, 35, -1}, {33, -1, -1, -1, 37, -1},
    {30, 35, 38, -1, -1, 34}, {32, 37, -1, -1, 39, 36},
    {-1, 39, -1, -1, -1, 38}};

struct StripConfig {
  byte channelIndex;
  byte topNodeIndex;
  byte bottomNodeIndex;
  uint16_t topLEDIndex;
  uint16_t bottomLEDIndex;
};

// First member: Strip number
// Second: LED index closer to ceiling
// Third: LED index closer to floor
StripConfig stripConfigs[40] = {
    {2, 0, 3, headof(3), tailof(3)},   {2, 0, 4, tailof(2), headof(2)},
    {1, 1, 4, headof(10), tailof(10)}, {1, 1, 5, tailof(9), headof(9)},
    {1, 2, 5, headof(4), tailof(4)},   {1, 2, 6, tailof(3), headof(3)},
    {2, 3, 7, tailof(6), headof(6)},   {3, 4, 7, tailof(11), headof(11)},
    {1, 4, 8, headof(11), tailof(11)}, {1, 5, 8, tailof(8), headof(8)},
    {1, 5, 9, headof(12), tailof(12)}, {0, 6, 9, tailof(11), headof(11)},
    {2, 3, 10, headof(4), tailof(4)},  {3, 7, 14, tailof(10), headof(10)},
    {2, 4, 11, tailof(1), headof(1)},  {1, 8, 15, tailof(7), headof(7)},
    {1, 5, 12, headof(5), tailof(5)},  {0, 9, 16, tailof(10), headof(10)},
    {1, 6, 13, tailof(2), headof(2)},  {2, 10, 14, headof(5), tailof(5)},
    {3, 11, 14, tailof(4), headof(4)}, {3, 11, 15, headof(5), tailof(5)},
    {0, 12, 15, headof(5), tailof(5)}, {0, 12, 16, tailof(4), headof(4)},
    {1, 13, 16, tailof(1), headof(1)}, {3, 14, 17, tailof(9), headof(9)},
    {0, 15, 17, headof(6), tailof(6)}, {1, 15, 18, tailof(6), headof(6)},
    {0, 16, 18, tailof(9), headof(9)}, {3, 14, 19, tailof(3), headof(3)},
    {3, 17, 22, tailof(8), headof(8)}, {3, 15, 20, headof(6), tailof(6)},
    {0, 18, 23, tailof(8), headof(8)}, {0, 16, 21, tailof(3), headof(3)},
    {3, 19, 22, tailof(2), headof(2)}, {3, 20, 22, headof(7), tailof(7)},
    {0, 20, 23, headof(7), tailof(7)}, {0, 21, 23, tailof(2), headof(2)},
    {3, 22, 24, tailof(1), headof(1)}, {0, 23, 24, tailof(1), headof(1)}};

// Border nodes are on the very edge of the network.
// Ripples fired here don't look very impressive.
int numberOfBorderNodes = 10;
int borderNodes[] = {0, 1, 2, 3, 6, 10, 13, 19, 21, 24};

// Cube nodes link three equiangular segments
// Firing ripples that always turn in one direction will draw a cube
int numberOfCubeNodes = 8;
int cubeNodes[] = {7, 8, 9, 11, 12, 17, 18};

// Firing ripples that always turn in one direction will draw a starburst
int starburstNode = 15;

#endif
