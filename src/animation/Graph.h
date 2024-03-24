#ifndef Graph_h
#define Graph_h

#include <FastLED.h>

#include "config.h"
#include "mapping.h"

class Node;

class Strip {
 public:
  int index;
  int length = STRIP_LED_COUNT;
  int channel;
  int channelLEDOffset;
  int channelOffset;
  int channelLength;
  int stripStartLED;
  int stripEndLED;

  Node *topNode;
  Node *bottomNode;

  Strip(){};
  Strip(int _index) {
    index = _index;
    channel = ledAssignments[index][0];
    channelLength = lengths[channel];
    channelOffset = channelOffsets[channel];
    stripStartLED = channelOffset + ledAssignments[index][1];
    stripEndLED = channelOffset + ledAssignments[index][2];
  }

  /** Get the global LED index for a given index on the strip */
  int getLEDIndex(int stripIndex) {
    return map(stripIndex, 0, 13, stripEndLED, stripStartLED);
  }

  /** Merge the provided colour into the target LED weighted by the provided
   * proportion */
  void applyColorToLED(CRGB ledColors[TOTAL_LEDS], int led, CRGB color,
                       float proportion) {
    int ledIndex = getLEDIndex(led);
    CRGB currentColour = ledColors[ledIndex];

    ledColors[ledIndex].setRGB(qadd8(currentColour.r, color.r * proportion),
                               qadd8(currentColour.g, color.g * proportion),
                               qadd8(currentColour.b, color.b * proportion));
  }
};

class Node {
 public:
  int index;
  Strip *strips[6];
  Node(){};
  Node(int index) { this->index = index; }
};

class Graph {
 public:
  Strip strips[STRIP_COUNT];
  Node nodes[NODE_COUNT];

  Graph() {
    // Create all of the strips
    for (int stripIndex = 0; stripIndex < STRIP_COUNT; stripIndex++) {
      strips[stripIndex] = Strip(stripIndex);
    }

    // Create all of the nodes
    for (int nodeIndex = 0; nodeIndex < NODE_COUNT; nodeIndex++) {
      nodes[nodeIndex] = Node(nodeIndex);
    }

    // Add all of the node references to each strip
    for (int stripIndex = 0; stripIndex < STRIP_COUNT; stripIndex++) {
      int topNodeIndex = segmentConnections[stripIndex][0];
      int bottomNodeIndex = segmentConnections[stripIndex][1];

      strips[stripIndex].topNode = &nodes[topNodeIndex];
      strips[stripIndex].bottomNode = &nodes[bottomNodeIndex];
    }

    // Add all of the strip references to each node
    for (int nodeIndex = 0; nodeIndex < NODE_COUNT; nodeIndex++) {
      for (int direction = 0; direction < 6; direction++) {
        int stripIndex = nodeConnections[nodeIndex][direction];
        if (stripIndex >= 0) {
          nodes[nodeIndex].strips[direction] = &strips[stripIndex];
        }
      }
    }
  }
};

Graph graph;

#endif