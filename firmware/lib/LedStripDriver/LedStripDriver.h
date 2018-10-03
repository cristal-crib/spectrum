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
  LedStripDriver(uint16_t pixelCount, uint8_t pin)
  {
    _stripDataPin = pin;
    SetPixelCount(pixelCount);
    strip->Begin();
  }

  void SetPixelCount(uint16_t pixelCount)
  {
    _pixelCount = pixelCount;
    if (strip != NULL)
    {
      delete strip;
    }
    strip = new NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod>(pixelCount, _stripDataPin);
  }

  void SetFullStripColor(RgbwColor color)
  {
    for (uint16_t i = 0; i < _pixelCount; i++)
    {
      strip->SetPixelColor(i, color);
    }
    strip->Show();
  }

  void ConfigureSegment(StripSegment segment)
  {
    _segmentLengths[segment.index] = segment.lenght;

    unsigned int sumCount = 0;
    for (size_t i = 0; i < 10; i++)
    {
      sumCount += _segmentLengths[i];
    }

    SetPixelCount(sumCount);
  }

protected:
  NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> *strip = NULL;
  uint16_t _pixelCount;
  uint8_t _stripDataPin;
  unsigned int _segmentLengths[10];
};
