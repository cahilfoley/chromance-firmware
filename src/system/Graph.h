#ifndef Graph_h
#define Graph_h

#include <FastLED.h>

#include "config.h"
#include "system/mapping.h"

class Strip;

class Node {
 public:
  byte index;
  Strip *strips[6];
  bool borderNode = false;
  byte row;
  byte column;
  int x;
  int y;

  Node(){};
  Node(byte index) {
    this->index = index;
    this->row = nodeConnections[index].row;
    this->column = nodeConnections[index].column;

    // Have to add one to the strip LED count to account for the node itself and any verticals in line with it
    this->x = this->column * (STRIP_LED_COUNT + 1);
    this->y = this->row * (STRIP_LED_COUNT + 1);
  }

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

struct Coordinate {
  int x;
  int y;
};

struct LEDCoordinate {
  int x;
  int y;
  int globalIndex;
};

void printColor(const CRGB *color) {
  Serial.print(color->r);
  Serial.print(", ");
  Serial.print(color->g);
  Serial.print(", ");
  Serial.print(color->b);
}

class Strip {
 public:
  byte index;
  byte length = STRIP_LED_COUNT;
  byte channel;
  byte row;
  byte column;
  StripOrientation orientation;
  LEDCoordinate leds[STRIP_LED_COUNT];
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
    orientation = stripConfigs[index].orientation;
    channel = stripConfigs[index].channelIndex;
    channelLength = lengths[channel];
    channelOffset = channelOffsets[channel];
    stripStartLED = channelOffset + stripConfigs[index].topLEDIndex;
    stripEndLED = channelOffset + stripConfigs[index].bottomLEDIndex;
  }

  // Calculate the coordinates of each LED in the strip, this should only be called once and should be called before
  // anything else
  void calculateLEDPositions() {
    for (byte i = 0; i < STRIP_LED_COUNT; i++) {
      auto coordinate = getLEDCoordinates(i);
      /** Get the global LED index for a given index on the strip, this is the position in the `leds` array. */
      int ledIndex = map(i, 0, length - 1, stripEndLED, stripStartLED);
      leds[i] = {coordinate.x, coordinate.y, ledIndex};
    }
  }

  /** Merge the provided colour into the target LED weighted by the provided
   * proportion */
  void applyColorToLED(CRGB ledColors[TOTAL_LEDS], int led, const CRGB *color, float proportion) {
    auto ledCoordinate = leds[led];
    auto currentColor = &ledColors[ledCoordinate.globalIndex];

#ifdef DEBUG_COLOR_MIXING
    Serial.print("Applying color to LED with existing color: ");
    printColor(currentColor);
    Serial.print(" and new color: ");
    printColor(color);
    Serial.print(" with proportion: ");
    Serial.print(proportion);
#endif

    // nblend(*currentColor, *color, proportion);
    currentColor->setRGB(qadd8(currentColor->r, float(color->r) * proportion),
                         qadd8(currentColor->g, float(color->g) * proportion),
                         qadd8(currentColor->b, float(color->b) * proportion));

#ifdef DEBUG_COLOR_MIXING
    Serial.print(" resulting in color: ");
    printColor(currentColor);
    Serial.println();
#endif
  }

 private:
  Coordinate getLEDCoordinates(int stripIndex) {
    int x;
    int y;
    if (this->orientation == vertical) {
      x = this->bottomNode->x;
      // The y value is the bottom node's y value plus the strip index * 2 to account for the vertical spacing
      y = this->topNode->y + (this->length - stripIndex) * 2;
    } else if (this->orientation == downRight) {
      x = this->topNode->x + this->length - stripIndex;
      y = this->bottomNode->y - stripIndex - 1;
    } else if (this->orientation == downLeft) {
      x = this->bottomNode->x + stripIndex + 1;
      y = this->bottomNode->y - stripIndex - 1;
    }

    return {x, y};
  }
};

class Graph {
 public:
  Strip strips[STRIP_COUNT];
  Node nodes[NODE_COUNT];
  LEDCoordinate *ledCoordinates[xLimit][yLimit];

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

    // Calculate the LED positions for each strip
    for (auto &strip : strips) {
      strip.calculateLEDPositions();

      // Populate each of the LEDs into the coordinate grid
      for (auto &led : strip.leds) {
        ledCoordinates[led.x][led.y] = &led;
      }
    }
  }
};

Graph graph;

#endif