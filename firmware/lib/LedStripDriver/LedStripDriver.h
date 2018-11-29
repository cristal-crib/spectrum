#pragma once

#include <NeoPixelBus.h>
#include "WebDriver.h"

static const RgbwColor Zero(HtmlColor(0x00000000));
static const RgbwColor White(HtmlColor(0xff000000));
static const RgbwColor Red(HtmlColor(0x00ff0000));
static const RgbwColor Green(HtmlColor(0x0000ff00));
static const RgbwColor Blue(HtmlColor(0x000000ff));
static const RgbwColor Candle(HtmlColor(0xFFFF0000));

class LedStripDriver
{
public:
  LedStripDriver(uint8_t pin)
  {
    _stripDataPin = pin;    
  }

  void SetPixelCount(uint16_t pixelCount)
  {
    _pixelCount = pixelCount;

    if (strip != NULL)
    {
      delete strip;
    }
    
    strip = new NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod>(pixelCount, _stripDataPin);
    strip->Begin();
  }

  void SetFullStripColor(RgbwColor color)
  {
    for (int i = 0; i < _pixelCount; i++)
    {
      strip->SetPixelColor(i, color);
    }
    strip->Show();
  }

  void SetSegmentColor(int segmentIndex, RgbwColor color)
  {
    if (_segmentLengths[segmentIndex] == 0)
    {
      Serial.println("Segment " + String(segmentIndex) + " is not configured and won't be set.");
      return;
    }

    int segmentBeginning = 0;    
    for(int i = 0; i < segmentIndex; i++)
    {
      segmentBeginning += _segmentLengths[i];
    }
    Serial.println("Segment " + String(segmentIndex) + " starts at " + String(segmentBeginning));
    
    int segmentLenght = _segmentLengths[segmentIndex];
    for (int i = segmentBeginning; i < segmentBeginning + segmentLenght; i++)
    {
      strip->SetPixelColor(i, color);
    }

    strip->Show();
  }

  void ConfigureSegment(StripSegment segment)
  {
    if (_segmentLengths[segment.index] > 0)
      SetSegmentColor(segment.index, Zero);

    _segmentLengths[segment.index] = segment.lenght;

    unsigned int sumCount = 0;
    for (int i = 0; i < 10; i++)
    {
      sumCount += _segmentLengths[i];
    }

    Serial.println("Total pixel count of all segment within the strip: " + String(sumCount));
    SetPixelCount(sumCount);
    SetSegmentColor(segment.index, White);
  }

protected:
  NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> *strip = NULL;
  uint16_t _pixelCount;
  uint8_t _stripDataPin;
  unsigned int _segmentLengths[10];
};
