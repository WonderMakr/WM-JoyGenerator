/*
||
|| @author         Brett Hagman <bhagman@roguerobotics.com>
|| @url            http://roguerobotics.com/
|| @url            http://wiring.org.co/
||
|| @description
|| |
|| | Joy Generator - Controller
|| |
|| #
||
|| @license
|| |
|| | Copyright (c) 2016 - Brett Hagman (http://roguerobotics.com/)
|| |
|| | Permission is hereby granted, free of charge, to any person obtaining a copy of
|| | this software and associated documentation files (the "Software"), to deal in
|| | the Software without restriction, including without limitation the rights to
|| | use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
|| | the Software, and to permit persons to whom the Software is furnished to do so,
|| | subject to the following conditions:
|| |
|| | The above copyright notice and this permission notice shall be included in all
|| | copies or substantial portions of the Software.
|| |
|| | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
|| | IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
|| | FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
|| | COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
|| | IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
|| | CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
|| #
||
|| @notes
|| |
|| | 3 x buttons, 1 x dial (with 6 x switches), 1 x lever (with 6 x switches)
|| | all switches are using a LitSwitch configuration -- 5v -> 330R -> i/o -> 330R -> GND
|| |
|| | Control   Pin
|| | Button1   0
|| | Button2   1
|| | Button3   2
|| | Lever1    3
|| | Lever2    4
|| | Lever3    5
|| | Lever4    6
|| | Lever5    7
|| | Lever6    8
|| | Dial1     14
|| | Dial2     15
|| | Dial3     16
|| | Dial4     17
|| | Dial5     18
|| | Dial6     19
|| |
|| | Protocol:
|| |
|| | CMDs
|| | - 'A' set animation number
|| | - 'a' set parameter 1 (usually primary colour)
|| | - 'b' set parameter 2 (usually speed)
|| | - 'c' set parameter 3 (usually secondary colour)
|| |
|| #
||
*/


#include <SPI.h>
#include "RF24.h"
#include "LitSwitch.h"

#define VERSIONSTR "1.3"

#define MYCONTROLLERNUMBER 0

#define DEBUG 0

#define PIN_BUTTON1   0
#define PIN_BUTTON2   1
#define PIN_BUTTON3   2
#define PIN_LEVER1    3
#define PIN_LEVER2    4
#define PIN_LEVER3    5
#define PIN_LEVER4    6
#define PIN_LEVER5    7
#define PIN_LEVER6    8
#define PIN_DIAL1     14
#define PIN_DIAL2     15
#define PIN_DIAL3     16
#define PIN_DIAL4     17
#define PIN_DIAL5     18
#define PIN_DIAL6     19

#define COLOURCOUNT 3

#define INDEX_BUTTON1 0
#define INDEX_BUTTON2 1
#define INDEX_BUTTON3 2
#define INDEX_LEVER   3
#define INDEX_DIAL    4

byte ctrlAddr[][6] = { "1Ctrl", "2Ctrl" };
byte treeAddr[][6] = { "1Tree", "2Tree", "3Tree" };

byte controlToTree[2][5] = { { 0, 1, 2, 0, 1 },
                             { 2, 1, 0, 2, 2 } };
//0x5D00B2 - purple
//0xB21700 - red
//0xFFDD00 - yellow
//0xFF6D00 - orange
//0x004EB2 - blue
//0x51FF00 - green
//
uint32_t primaryColour[] = { 0xFF0000, 0xB21700, 0xFF6D00, 0x0000FF, 0x00FF00, 0x993311 };
uint32_t secondaryColour[] = { 0x5D00B2, 0x004EB2, 0x51FF00, 0xE100CC, 0xC9C901, 0xFF0C0C };

// uint8_t speeds[] = { 1, 4, 8, 15, 20, 30 };
uint8_t speeds[] = { 30, 20, 15, 8, 4, 1 };

RF24 radio(9, 10);

LitSwitch button1 = LitSwitch(PIN_BUTTON1, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch button2 = LitSwitch(PIN_BUTTON2, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch button3 = LitSwitch(PIN_BUTTON3, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch lever1 = LitSwitch(PIN_LEVER1, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch lever2 = LitSwitch(PIN_LEVER2, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch lever3 = LitSwitch(PIN_LEVER3, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch lever4 = LitSwitch(PIN_LEVER4, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch lever5 = LitSwitch(PIN_LEVER5, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch lever6 = LitSwitch(PIN_LEVER6, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch dial1 = LitSwitch(PIN_DIAL1, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch dial2 = LitSwitch(PIN_DIAL2, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch dial3 = LitSwitch(PIN_DIAL3, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch dial4 = LitSwitch(PIN_DIAL4, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch dial5 = LitSwitch(PIN_DIAL5, CATHODE, PULLUP_INTERNAL, 50);
LitSwitch dial6 = LitSwitch(PIN_DIAL6, CATHODE, PULLUP_INTERNAL, 50);


/**
* Create a data structure for transmitting and receiving data
* This allows many variables to be easily sent and received in a single transmission
* See http://www.cplusplus.com/doc/tutorial/structures/
*/
struct dataStruct
{
  char cmd;
  uint32_t value;
} myData;


void writeToTree(uint8_t treeNumber, char command, uint32_t data)
{
  dataStruct sendData;

  // Bail, if the treeNumber is out of range.
  if (treeNumber > 2)
    return;

#if DEBUG
Serial.print(F("Sending "));
Serial.print(command);
Serial.print(' ');
Serial.print(data, HEX);
Serial.print(F(" to tree #"));
Serial.print(treeNumber);
#endif

  radio.stopListening();                                    // First, stop listening so we can talk.
  radio.openWritingPipe(treeAddr[treeNumber]);

  sendData.cmd = command;
  sendData.value = data;

  if (radio.write(&sendData, sizeof(sendData)))
  {
#if DEBUG
    Serial.println(F(" -- Send success."));
#endif
  }
  else
  {
#if DEBUG
    Serial.println(F(" -- Send failed!"));
#endif
  }

  radio.startListening();                                    // Now, continue listening
}


void onPressButton1(LitSwitch *btn)
{
  static uint8_t colourIndex = 0;
  uint32_t colour;

#if MYCONTROLLERNUMBER == 0
  colour = primaryColour[colourIndex];
  writeToTree(controlToTree[MYCONTROLLERNUMBER][INDEX_BUTTON1], 'a', colour);
#else
  colour = secondaryColour[colourIndex];
  writeToTree(controlToTree[MYCONTROLLERNUMBER][INDEX_BUTTON1], 'c', colour);
#endif
  colourIndex++;
  if (colourIndex >= COLOURCOUNT)
    colourIndex = 0;
}


void onPressButton2(LitSwitch *btn)
{
  static uint8_t colourIndex = 0;
  uint32_t colour;

#if MYCONTROLLERNUMBER == 0
  colour = primaryColour[colourIndex];
  writeToTree(controlToTree[MYCONTROLLERNUMBER][INDEX_BUTTON2], 'a', colour);
#else
  colour = secondaryColour[colourIndex];
  writeToTree(controlToTree[MYCONTROLLERNUMBER][INDEX_BUTTON2], 'c', colour);
#endif
  colourIndex++;
  if (colourIndex >= COLOURCOUNT)
    colourIndex = 0;
}


void onPressButton3(LitSwitch *btn)
{
  static uint8_t colourIndex = 0;
  uint32_t colour;

#if MYCONTROLLERNUMBER == 0
  colour = primaryColour[colourIndex];
  writeToTree(controlToTree[MYCONTROLLERNUMBER][INDEX_BUTTON3], 'a', colour);
#else
  colour = secondaryColour[colourIndex];
  writeToTree(controlToTree[MYCONTROLLERNUMBER][INDEX_BUTTON3], 'c', colour);
#endif
  colourIndex++;
  if (colourIndex >= COLOURCOUNT)
    colourIndex = 0;
}


void setAnimation(uint8_t treeNumber, uint8_t animationNumber)
{
  writeToTree(treeNumber, 'A', animationNumber);
}


void setSpeed(uint8_t treeNumber, uint8_t speed)
{
  writeToTree(treeNumber, 'b', speed);
}


void onPressDial1(LitSwitch *btn)
{
#if MYCONTROLLERNUMBER == 0
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_DIAL], 0);
#else
  setSpeed(0, speeds[0]);
  setSpeed(1, speeds[0]);
  setSpeed(2, speeds[0]);
#endif
}

void onPressDial2(LitSwitch *btn)
{
#if MYCONTROLLERNUMBER == 0
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_DIAL], 1);
#else
  setSpeed(0, speeds[1]);
  setSpeed(1, speeds[1]);
  setSpeed(2, speeds[1]);
#endif
}

void onPressDial3(LitSwitch *btn)
{
#if MYCONTROLLERNUMBER == 0
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_DIAL], 2);
#else
  setSpeed(0, speeds[2]);
  setSpeed(1, speeds[2]);
  setSpeed(2, speeds[2]);
#endif
}

void onPressDial4(LitSwitch *btn)
{
#if MYCONTROLLERNUMBER == 0
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_DIAL], 3);
#else
  setSpeed(0, speeds[3]);
  setSpeed(1, speeds[3]);
  setSpeed(2, speeds[3]);
#endif
}

void onPressDial5(LitSwitch *btn)
{
#if MYCONTROLLERNUMBER == 0
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_DIAL], 4);
#else
  setSpeed(0, speeds[4]);
  setSpeed(1, speeds[4]);
  setSpeed(2, speeds[4]);
#endif
}

void onPressDial6(LitSwitch *btn)
{
#if MYCONTROLLERNUMBER == 0
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_DIAL], 5);
#else
  setSpeed(0, speeds[5]);
  setSpeed(1, speeds[5]);
  setSpeed(2, speeds[5]);
#endif
}


void onPressLever1(LitSwitch *btn)
{
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_LEVER], 0);
}

void onPressLever2(LitSwitch *btn)
{
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_LEVER], 1);
}

void onPressLever3(LitSwitch *btn)
{
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_LEVER], 2);
}

void onPressLever4(LitSwitch *btn)
{
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_LEVER], 3);
}

void onPressLever5(LitSwitch *btn)
{
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_LEVER], 4);
}

void onPressLever6(LitSwitch *btn)
{
  setAnimation(controlToTree[MYCONTROLLERNUMBER][INDEX_LEVER], 5);
}

void setup()
{
#if DEBUG
  Serial.begin(115200);
  Serial.println(F("JoyGen -- Controller V" VERSIONSTR));
  Serial.print(F("My controller number: "));
  Serial.println(MYCONTROLLERNUMBER);
#else
  Serial.end();
#endif

  button1.onPress = onPressButton1;
  button2.onPress = onPressButton2;
  button3.onPress = onPressButton3;

  dial1.onPress = onPressDial1;
  dial2.onPress = onPressDial2;
  dial3.onPress = onPressDial3;
  dial4.onPress = onPressDial4;
  dial5.onPress = onPressDial5;
  dial6.onPress = onPressDial6;

  lever1.onPress = onPressLever1;
  lever2.onPress = onPressLever2;
  lever3.onPress = onPressLever3;
  lever4.onPress = onPressLever4;
  lever5.onPress = onPressLever5;
  lever6.onPress = onPressLever6;

  radio.begin();

  radio.setPALevel(RF24_PA_MAX);

  radio.openWritingPipe(treeAddr[0]);
  radio.openReadingPipe(1, ctrlAddr[MYCONTROLLERNUMBER]);

  radio.startListening();
}


void loop()
{

  LitSwitch::updateAll();

/*
  uint16_t reading;
  static uint32_t lastTime = 0;
  static uint16_t lastReading = 0;

  static uint8_t testTree = 0;

  char c;

  if ((millis() - lastTime) > 10)  // read only every x ms
  {
    lastTime = millis();
    reading = map(analogRead(0), 0, 1023, 0, 100);
    if (reading != lastReading)
    {
      lastReading = reading;

#if DEBUG
      Serial.print(F("Changed to "));
      Serial.println(reading);
#endif

      writeToTree(testTree, 'b', reading);
    }
  }

  if (Serial.available())
  {
    c = Serial.read();

    switch (c)
    {
      case '1':
        writeToTree(testTree, 'A', 0);
        break;
      case '2':
        writeToTree(testTree, 'A', 1);
        break;
      case '3':
        writeToTree(testTree, 'A', 2);
        break;
      case '4':
        writeToTree(testTree, 'A', 3);
        break;
      case '5':
        writeToTree(testTree, 'A', 4);
        break;
      case '6':
        writeToTree(testTree, 'A', 5);
        break;
      case 'r':
        writeToTree(testTree, 'a', 0xff0000);
        writeToTree(testTree, 'c', 0x00ff00); // green
        break;
      case 'g':
        writeToTree(testTree, 'a', 0x00ff00);
        writeToTree(testTree, 'c', 0xffffff); // white
        break;
      case 'b':
        writeToTree(testTree, 'a', 0x0000ff);
        writeToTree(testTree, 'c', 0xffff00); // yellow
        break;
      case 'X':
        writeToTree(testTree, 'b', 10);
        break;
      case 'Y':
        writeToTree(testTree, 'b', 20);
        break;
      case 'Z':
        writeToTree(testTree, 'b', 50);
        break;
      case 'u':
        testTree = 0;
        break;
      case 'i':
        testTree = 1;
        break;
      case 'o':
        testTree = 2;
        break;
    }
  }
*/
}

