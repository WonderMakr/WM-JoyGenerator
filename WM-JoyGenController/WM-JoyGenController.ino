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

#define VERSIONSTR "1.0"

#define DEBUG 1

byte ctrlAddr[][6] = { "1Ctrl", "2Ctrl" };
byte treeAddr[][6] = { "1Tree", "2Tree", "3Tree" };

uint8_t myControllerNumber = 0;

RF24 radio(9, 10);

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


void setup()
{
  Serial.begin(115200);
  Serial.println(F("JoyGen -- Controller V" VERSIONSTR));
  Serial.print(F("My controller number: "));
  Serial.println(myControllerNumber);

  radio.begin();

  radio.setPALevel(RF24_PA_MAX);

  radio.openWritingPipe(treeAddr[0]);
  radio.openReadingPipe(1, ctrlAddr[myControllerNumber]);

  radio.startListening();
}


void loop()
{
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

}

