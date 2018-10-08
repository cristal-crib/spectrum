#pragma once

#include <LedStripDriver.h>
#include <Preferences.h>

class LedStripManager
{
public:
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
  static StripSegmentState _receivedState;
  static StripSegment _receivedSegment;

  static void Loop(void *parameter)
  {
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
        _ledStripDriver->ConfigureSegment(_receivedSegment);

        try 
        { 
          Preferences preferences;
          preferences.begin("segment-config");

          char ab[10];  //allocate memory
          sprintf(ab, "segment%d", _receivedSegment.index);

          Serial.println("Writing segment preference to " + String(ab) + " with value of " + String(_receivedSegment.lenght));
          preferences.putUInt(ab, _receivedSegment.lenght);

          unsigned int length = preferences.getUInt(ab, 0);
          Serial.println("Readback gave the following value " + String(length));

          preferences.end();
        } 
        catch (const std::exception& e) 
        {
          Serial.println(e.what());
        }

        
      }

      delay(50);
    }
  }
};

QueueHandle_t LedStripManager::_stripCommandQueue;
QueueHandle_t LedStripManager::_stripConfigQueue;
LedStripDriver *LedStripManager::_ledStripDriver;
StripSegmentState LedStripManager::_receivedState;
StripSegment LedStripManager::_receivedSegment;
