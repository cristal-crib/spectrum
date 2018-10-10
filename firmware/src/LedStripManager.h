#pragma once

#include <LedStripDriver.h>
#include <Preferences.h>

class LedStripManager
{
public:
  const static char* PreferenceNamespace;

  LedStripManager()
  {
  }

  void Init(QueueHandle_t stripCommandQueue, QueueHandle_t stripConfigQueue, LedStripDriver *ledStripDriver)
  {
    _stripCommandQueue = stripCommandQueue;
    _stripConfigQueue = stripConfigQueue;
    _ledStripDriver = ledStripDriver;

    xTaskCreate(
        Loop,                  /* Task function. */
        "LedStripManagerLoop", /* String with name of task. */
        5000,                  /* Stack size in words. */
        NULL,                  /* Parameter passed as input of the task */
        1,                     /* Priority of the task. */
        NULL);                 /* Task handle. */
  }

private:
  static QueueHandle_t _stripCommandQueue;
  static QueueHandle_t _stripConfigQueue;
  static LedStripDriver *_ledStripDriver;

  static void Loop(void *parameter)
  {
    StripSegmentState _receivedState;
    StripSegment _receivedSegment;
    while (1)
    {
      if (xQueueReceive(_stripCommandQueue, &_receivedState, 0))
      {
        Serial.println("StripManager: Received new strip command: H " + String(_receivedState.hue) + " S " + String(_receivedState.saturation) + " B " + String(_receivedState.brightness));
        if (_receivedState.onOff == 0)
        {
          _ledStripDriver->SetFullStripColor(Zero);
        }
        else
        {
          HsbColor color(_receivedState.hue / 360.0f, _receivedState.saturation/100.0f, _receivedState.brightness/100.f);
          _ledStripDriver->SetFullStripColor(color);
        }
      }

      if (xQueueReceive(_stripConfigQueue, &_receivedSegment, 0))
      {
        Serial.println("StripManager: Received new strip configuration: Index " + String(_receivedSegment.index) + " Length " + String(_receivedSegment.lenght));
        SaveSegmentConfguration(_receivedSegment);
        _ledStripDriver->ConfigureSegment(_receivedSegment);        
      }

      delay(50);
    }
  }

  static void SaveSegmentConfguration(StripSegment stripSegment)
  {
    char segmentKey[10];
    sprintf(segmentKey, "segment%d", stripSegment.index);

    Preferences preferences;
    preferences.begin(PreferenceNamespace);
    preferences.putUInt(segmentKey, stripSegment.lenght);
    preferences.end();
  }

  static unsigned int ReadSegmentConfiguration(int index)
  {
    char segmentKey[10];
    sprintf(segmentKey, "segment%d", index);

    Preferences preferences;
    preferences.begin(PreferenceNamespace);
    unsigned int length = preferences.getUInt(segmentKey, -1);
    preferences.end();

    return length;
  }
};

QueueHandle_t LedStripManager::_stripCommandQueue;
QueueHandle_t LedStripManager::_stripConfigQueue;
LedStripDriver *LedStripManager::_ledStripDriver;

const char* LedStripManager::PreferenceNamespace = "segment-config";
