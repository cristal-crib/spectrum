#pragma once

#include <Arduino.h>

enum LedAnimationState
{
  Off = 0,
  Solid = 1,
  Breathe = 2,
  Pulse = 3
};

struct LedColor_t
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

#if BOARD_VERSION == 1
#define STATUS_LED_RED_PIN 26
#define STATUS_LED_GREEN_PIN 27
#define STATUS_LED_BLUE_PIN 25
#elif BOARD_VERSION == 2
#define STATUS_LED_RED_PIN 14
#define STATUS_LED_GREEN_PIN 12
#define STATUS_LED_BLUE_PIN 27
#endif

LedAnimationState _animationState;
LedColor_t _ledColor;

uint _animationFrameCouter;

class StatusLedDriver
{
private:
  // LedC Config
  const int freq = 5000;
  static const uint8_t _redChannel = 0;
  static const uint8_t _greenChannel = 1;
  static const uint8_t _blueChannel = 2;
  static const uint8_t resolution = 8;

  static const uint _frameTime = 20;
  static const uint _pulseHighDelay = 20;
  static const uint _pulseLoopDelay = 400;

  static void LedLoop(void *parameter)
  {
    while (1)
    {

      switch (_animationState)
      {
      case Solid:
        WriteLedValue(_ledColor);
        break;
      case Breathe:
        break;
      case Pulse:
        ExecutePulseFrame();
        break;
      case Off:
      default:
        WriteLedValue(0, 0, 0);
        break;
      }
      // 60 FPS!!
      _animationFrameCouter++;
      delay(_frameTime);
    }
  }

  static void ExecutePulseFrame()
  {
    auto isHigh = _animationFrameCouter % (_pulseLoopDelay / _frameTime) <= (_pulseHighDelay / _frameTime);
    if (isHigh)
    {
      WriteLedValue(_ledColor);
    }
    else
    {
      WriteLedValue(0, 0, 0);
    }
  }

  static void WriteLedValue(uint8_t red, uint8_t green, uint8_t blue)
  {
    ledcWrite(_redChannel, red);
    ledcWrite(_greenChannel, green);
    ledcWrite(_blueChannel, blue);
  }

  static void WriteLedValue(LedColor_t color)
  {
    ledcWrite(_redChannel, color.red);
    ledcWrite(_greenChannel, color.green);
    ledcWrite(_blueChannel, color.blue);
  }

public:
  StatusLedDriver(/* args */)
  {
    _animationState = Solid;
    _ledColor.red = 0;
    _ledColor.green = 40;
    _ledColor.blue = 100;

    ledcSetup(_redChannel, freq, resolution);
    ledcSetup(_greenChannel, freq, resolution);
    ledcSetup(_blueChannel, freq, resolution);

    ledcAttachPin(STATUS_LED_RED_PIN, _redChannel);
    ledcAttachPin(STATUS_LED_GREEN_PIN, _greenChannel);
    ledcAttachPin(STATUS_LED_BLUE_PIN, _blueChannel);
  };

  void Init()
  {
    xTaskCreate(
        LedLoop,   /* Task function. */
        "LedLoop", /* String with name of task. */
        1000,      /* Stack size in words. */
        NULL,      /* Parameter passed as input of the task */
        1,         /* Priority of the task. */
        NULL);     /* Task handle. */
  };

  void SetLedState(LedAnimationState state)
  {
    _animationState = state;
  };

  void SetLedState(LedAnimationState state, LedColor_t color)
  {
    _animationState = state;
    _ledColor = color;
  };

  void SetLedState(LedAnimationState state, uint8_t red, uint8_t green, uint8_t blue)
  {
    _animationState = state;
    _ledColor.red = red;
    _ledColor.green = green;
    _ledColor.blue = blue;
  };

  ~StatusLedDriver(){};
};
