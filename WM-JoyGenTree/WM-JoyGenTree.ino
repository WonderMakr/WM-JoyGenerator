/*
||
|| @author         Brett Hagman <bhagman@roguerobotics.com>
|| @url            http://roguerobotics.com/
|| @url            http://wiring.org.co/
||
|| @description
|| |
|| | Joy Generator - Receiver/Tree
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
#include <Adafruit_NeoPixel.h>

#define VERSIONSTR "1.1"

#define MYTREENUMBER 0

#define DEBUG 1

#define PIN 6

#define IDLETIMEOUT 20000L

#define IDLECOLOUR 0x706010
#define IDLESPARKLECOLOUR 0xffffff
#define IDLENEXTSPARKLETIME 50

#define PARAM1_DEFAULT 0xff0000
#define PARAM2_DEFAULT 20
#define PARAM3_DEFAULT 0x00ff00

byte ctrlAddr[][6] = { "1Ctrl", "2Ctrl" };
byte treeAddr[][6] = { "1Tree", "2Tree", "3Tree" };

RF24 radio(9, 10);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, PIN, NEO_RGB + NEO_KHZ800);

uint8_t animationNumber = 0;
uint32_t param1 = PARAM1_DEFAULT;
uint32_t param2 = PARAM2_DEFAULT;
uint32_t param3 = PARAM3_DEFAULT;

bool prepared = false;
uint32_t lastInteractionTime = 0;
bool idle = false;

struct dataStruct
{
  char cmd;
  uint32_t value;
} myData;


uint8_t red(uint32_t c)
{
  return (c >> 16);
}


uint8_t green(uint32_t c)
{
  return (c >> 8);
}


uint8_t blue(uint32_t c)
{
  return (c);
}


uint32_t fadeOne(uint32_t c, float rate)
{
  uint16_t r, g, b;

  r = constrain(red(c) * rate, 0, 255);
  g = constrain(green(c) * rate, 0, 255);
  b = constrain(blue(c) * rate, 0, 255);

  return strip.Color(r, g, b);
}


void fadeAll(float rate)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, fadeOne(strip.getPixelColor(i), rate));
  }
}


void prepareIdleAnimation()
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, IDLECOLOUR);
  }

  strip.show();
}


void idleAnimation()
{
  static uint32_t lastTime = 0;
  static uint8_t lastSparklePixel = 0;

  if (!prepared)
  {
    prepareIdleAnimation();
    prepared = true;
    lastTime = millis();
  }
/*
  if ((millis() - lastTime) > IDLENEXTSPARKLETIME)
  {
    strip.setPixelColor(lastSparklePixel, IDLECOLOUR);
    lastSparklePixel = random(strip.numPixels());
    strip.setPixelColor(lastSparklePixel, IDLESPARKLECOLOUR);
    strip.show();
    lastTime = millis();
  }
*/
}


// Animation 1: Barber pole - 64 Bit
// Param 1: colour
// Param 2: speed
// Param 3: secondary colour

void prepareAnimation1()
{
  strip.clear();
  strip.show();
}


void shiftAllUp30()
{
  for (int i = strip.numPixels() - 1; i > 29; i--)
  {
    strip.setPixelColor(i, strip.getPixelColor(i - 30));
  }
}


void animation1()
{
  static uint32_t lastTime = 0;
  static uint8_t lastColour = 0;
  uint32_t colourSet[] = { param1, IDLECOLOUR, param3, IDLECOLOUR };

  if (!prepared)
  {
    prepareAnimation1();
    prepared = true;
    lastTime = millis();
    lastColour = 0;
  }

  if ((millis() - lastTime) > param2 * 10 + 100)
  {
    shiftAllUp30();
    for (int i = 0; i < 30; i++)
    {
      strip.setPixelColor(i, colourSet[lastColour]);
    }
    strip.show();
    lastColour++;
    if (lastColour > 3)
      lastColour = 0;
    lastTime = millis();
  }
}


// Animation 3: Barber Pole - 8 Bit
// Param 1: colour
// Param 2: speed
// Param 3: secondary colour

void prepareAnimation3()
{
  strip.clear();

  strip.show();
}


void shiftAllUp()
{
  for (int i = strip.numPixels() - 1; i > 0; i--)
  {
    strip.setPixelColor(i, strip.getPixelColor(i - 1));
  }
}


void shiftAllDown()
{
  for (int i = 0; i < strip.numPixels() - 1; i++)
  {
    strip.setPixelColor(i, strip.getPixelColor(i + 1));
  }
}


void animation3()
{
  static uint32_t lastTime = 0;
  static uint8_t lastColour = 0;
  static uint8_t paintCount = 0;
  uint32_t colourSet[] = { param1, IDLECOLOUR, param3, IDLECOLOUR };

  if (!prepared)
  {
    prepareAnimation3();
    prepared = true;
    lastTime = millis();
    lastColour = 0;
    paintCount = 0;
  }

  if ((millis() - lastTime) > param2)
  {
    shiftAllDown();
    strip.setPixelColor(strip.numPixels() - 1, colourSet[lastColour]);
    strip.show();
    paintCount++;
    if (paintCount > 50)
    {
      paintCount = 0;
      lastColour++;
      if (lastColour > 3)
        lastColour = 0;
    }
    lastTime = millis();
  }
}


// Animation 2: random pixels with fade
// Param 1: colour
// Param 2: speed
// Param 3: secondary colour

void prepareAnimation2()
{
  strip.clear();
  strip.show();
}


void animation2()
{
  static uint32_t lastTime = 0;
  static bool odd = false;

  if (!prepared)
  {
    prepareAnimation2();
    prepared = true;
    lastTime = millis();
  }

  if ((millis() - lastTime) > param2)
  {
    fadeAll(0.98);
    odd = !odd;
    if (odd)
    {
      strip.setPixelColor(random(strip.numPixels()), param1);
    }
    else
    {
      strip.setPixelColor(random(strip.numPixels()), param3);
    }
    strip.show();
    lastTime = millis();
  }
}


// Animation 4: Alternating colours
// Param 1: colour
// Param 2: speed
// Param 3: secondary colour

void prepareAnimation4()
{
  strip.clear();
  strip.show();
}


void animation4()
{
  static uint32_t lastTime = 0;
  static uint8_t fadeCount = 0;
  static bool odd = false;
  uint32_t colour1;
  uint32_t colour2;

  if (!prepared)
  {
    prepareAnimation4();
    prepared = true;
    lastTime = millis();
  }

  if ((millis() - lastTime) > param2)
  {
//    fadeAll(0.95);
    fadeCount++;
    if (fadeCount >= 20)
    {
      fadeCount = 0;
      odd = !odd;
      if (odd)
      {
        colour1 = param3;
        colour2 = param1;
      }
      else
      {
        colour1 = param1;
        colour2 = param3;
      }

      for (int i = 0; i < strip.numPixels(); i++)
      {
        if (i & 0x1)
        {
          strip.setPixelColor(i, colour1);
        }
        else
        {
          strip.setPixelColor(i, colour2);
        }
      }
    }
    strip.show();
    lastTime = millis();
  }
}


// Animation 0: fill tree from bottom to top
// Param 1: colour
// Param 2: speed
// Param 3: secondary colour

void prepareAnimation0()
{
  strip.clear();
  strip.show();
}


void animation0()
{
  static uint32_t lastTime = 0;
  static uint8_t lastPixel = 0;
//  static uint8_t fadeCount = 0;  // Number of passes to fade, before beginning again
  static bool odd = false;
  uint32_t colour;

  if (!prepared)
  {
    prepareAnimation0();
    prepared = true;
    lastTime = millis();
    lastPixel = 0;
//    fadeCount = 0;
  }

  if ((millis() - lastTime) > param2)
  {
    if (lastPixel < strip.numPixels())
    {
      if (odd)
      {
        colour = param1;
      }
      else
      {
        colour = param3;
      }
      strip.setPixelColor(lastPixel, colour);
      lastPixel = lastPixel + 1;
    }
    else
    {
//      fadeCount++;
//      fadeAll(0.85);
//      if (fadeCount >= 10)
//      {
        // reset and begin again
//        fadeCount = 0;
        lastPixel = 0;
//        strip.clear();
        odd = !odd;
//      }
    }
    strip.show();
    lastTime = millis();
  }
}


// Animation 5: fill tree from top to bottom
// Param 1: colour
// Param 2: speed
// Param 3: secondary colour

void prepareAnimation5()
{
  strip.clear();
  strip.show();
}


void animation5()
{
  static uint32_t lastTime = 0;
  static uint8_t lastPixel = 0;
//  static uint8_t fadeCount = 0;  // Number of passes to fade, before beginning again
  static bool odd = false;
  uint32_t colour;

  if (!prepared)
  {
    prepareAnimation5();
    prepared = true;
    lastTime = millis();
    lastPixel = 0;
//    fadeCount = 0;
  }

  if ((millis() - lastTime) > param2)
  {
    if (lastPixel < strip.numPixels())
    {
      if (odd)
      {
        colour = param1;
      }
      else
      {
        colour = param3;
      }
      strip.setPixelColor(strip.numPixels() - lastPixel - 1, colour);
      lastPixel = lastPixel + 1;
    }
    else
    {
//      fadeCount++;
//      fadeAll(0.85);
//      if (fadeCount >= 10)
//      {
        // reset and begin again
//        fadeCount = 0;
        lastPixel = 0;
//        strip.clear();
        odd = !odd;
//      }
    }
    strip.show();
    lastTime = millis();
  }
}


void setIdleMode()
{
  idle = true;
  prepared = false;
}


void setActiveMode()
{
  idle = false;
  prepared = false;
  lastInteractionTime = millis();
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void setup()
{
  Serial.begin(115200);
  Serial.println(F("JoyGen -- Tree/Receiver V" VERSIONSTR));
  Serial.print(F("My tree number: "));
  Serial.println(MYTREENUMBER);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  radio.begin();

  radio.setPALevel(RF24_PA_MIN);

  radio.openWritingPipe(ctrlAddr[0]);
  radio.openReadingPipe(1, treeAddr[MYTREENUMBER]);

  radio.startListening();

  setIdleMode();
}


void loop()
{
  // If we aren't in idle mode, and comms have been quiet for some time, enter idle mode.
  if (!idle && (millis() - lastInteractionTime) > IDLETIMEOUT)
  {
    setIdleMode();
  }

  // Perform our animations
  if (idle)
  {
    idleAnimation();
  }
  else
  {
    switch (animationNumber)
    {
      case 0:
        animation0();
        break;
      case 1:
        animation1();
        break;
      case 2:
        animation2();
        break;
      case 3:
        animation3();
        break;
      case 4:
        animation4();
        break;
      case 5:
        animation5();
        break;
      default:
        break;
    }
  }


  // Check for comms
  if (radio.available())
  {
    int n = radio.available();

    while (radio.available())
    {
      radio.read(&myData, sizeof(myData));             // Get the payload
    }

#if DEBUG
    Serial.print(F("Got: "));
    Serial.print(myData.cmd);
    Serial.print(' ');
    Serial.println(myData.value);
#endif

    switch (myData.cmd)
    {
      case 'A':
        animationNumber = constrain(myData.value, 0, 5);
        if (idle)
          setActiveMode();
        else
          lastInteractionTime = millis();
        break;
      case 'a':
        param1 = myData.value;
        if (idle)
          setActiveMode();
        else
          lastInteractionTime = millis();
        break;
      case 'b':
        param2 = constrain(myData.value, 1, 1000);
        if (idle)
          setActiveMode();
        else
          lastInteractionTime = millis();
        break;
      case 'c':
        param3 = myData.value;
        if (idle)
          setActiveMode();
        else
          lastInteractionTime = millis();
        break;
      default:
        break;
    }
  }
}

