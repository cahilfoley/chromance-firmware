#ifndef Graph_h
#define Graph_h

#include <FastLED.h>

#include "config.h"
#include "system/mapping.h"

class Node;

class Strip {
 public:
  byte index;
  byte length = STRIP_LED_COUNT;
  byte channel;
  byte row;
  byte column;
  int channelLEDOffset;
  int channelOffset;
  int channelLength;
  int stripStartLED;
  int stripEndLED;

  Node *topNode;
  Node *bottomNode;

  Strip(){};
  Strip(byte _index) {
    index = _index;
    row = stripConfigs[index].row;
    column = stripConfigs[index].column;
    channel = stripConfigs[index].channelIndex;
    channelLength = lengths[channel];
    channelOffset = channelOffsets[channel];
    stripStartLED = channelOffset + stripConfigs[index].topLEDIndex;
    stripEndLED = channelOffset + stripConfigs[index].bottomLEDIndex;
  }

  /** Get the global LED index for a given index on the strip */
  int getLEDIndex(int stripIndex) { return map(stripIndex, 0, length - 1, stripEndLED, stripStartLED); }

  /** Merge the provided colour into the target LED weighted by the provided
   * proportion */
  void applyColorToLED(CRGB ledColors[TOTAL_LEDS], int led, CRGB color, float proportion) {
    int ledIndex = getLEDIndex(led);
    CRGB currentColour = ledColors[ledIndex];

    ledColors[ledIndex].setRGB(qadd8(currentColour.r, color.r * proportion),
                               qadd8(currentColour.g, color.g * proportion),
                               qadd8(currentColour.b, color.b * proportion));
  }
};

class Node {
 public:
  byte index;
  Strip *strips[6];
  bool borderNode = false;
  Node(){};
  Node(byte index) { this->index = index; }

  byte getStripCount() {
    byte count = 0;
    for (byte i = 0; i < 6; i++) {
      if (strips[i] != nullptr) {
        count++;
      }
    }
    return count;
  }

  void calculateProperties() {
    if (this->getStripCount() == 2) {
      borderNode = true;
    }
  }
};

class Graph {
 public:
  Strip strips[STRIP_COUNT];
  Node nodes[NODE_COUNT];
  // Strip *stripCoords[stripRowCount][stripColumnCount];

  Graph() {
    // Create all of the strips
    for (int stripIndex = 0; stripIndex < STRIP_COUNT; stripIndex++) {
      strips[stripIndex] = Strip(stripIndex);
      // stripCoords[strip.row][strip.column] = &strip;
    }

    // Create all of the nodes
    for (int nodeIndex = 0; nodeIndex < NODE_COUNT; nodeIndex++) {
      nodes[nodeIndex] = Node(nodeIndex);
    }

    // Add all of the node references to each strip
    for (byte stripIndex = 0; stripIndex < STRIP_COUNT; stripIndex++) {
      auto ledAssignment = stripConfigs[stripIndex];
      strips[stripIndex].topNode = &nodes[ledAssignment.topNodeIndex];
      strips[stripIndex].bottomNode = &nodes[ledAssignment.bottomNodeIndex];
    }

    // Add all of the strip references to each node
    for (int nodeIndex = 0; nodeIndex < NODE_COUNT; nodeIndex++) {
      for (int direction = 0; direction < 6; direction++) {
        int stripIndex = nodeConnections[nodeIndex].stripsIndexes[direction];
        if (stripIndex >= 0) {
          nodes[nodeIndex].strips[direction] = &strips[stripIndex];
        }
      }
    }

    // Do any post assembly property calculations (like figuring out border nodes)
    for (auto &node : nodes) {
      node.calculateProperties();
    }
  }
};

Graph graph;

#endif