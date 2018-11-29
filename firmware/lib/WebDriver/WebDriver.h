#pragma once

#include <WebServer.h>
#include <string>
#include "ESPAsyncWebServer.h"

struct StripSegmentState
{
  unsigned int index;
  unsigned int onOff;
  unsigned int hue;
  unsigned int saturation;
  unsigned int brightness;
};

struct StripSegment
{
  unsigned int index;
  unsigned int lenght;
};

class WebDriver
{
public:
  WebDriver()
  {
  }

  void Init(QueueHandle_t stripCommandQueue, QueueHandle_t stripConfigQueue)
  {
    _stripCommandQueue = stripCommandQueue;
    _stripConfigQueue = stripConfigQueue;

    for (unsigned int i = 0; i < 10; i++)
    {
      _stripState[i].onOff = 1;
      _stripState[i].brightness = 100;
      _stripState[i].saturation = 100;
      _stripState[i].hue = 225;
      _stripState[i].index = i;
    }

    _webServer = new AsyncWebServer(80);

    _webServer->on("/status", [&](AsyncWebServerRequest *request)
    {
      request->send(200, "text/plain", String(_stripState[GetIndex(request)].onOff));
    });

    _webServer->on("/on", [&](AsyncWebServerRequest *request)
    {
      Serial.println("Received a On command.");
      int segmentIndex = GetIndex(request);
      _stripState[segmentIndex].onOff = 1;
      xQueueSend(_stripCommandQueue, &_stripState[GetIndex(request)], portMAX_DELAY);
      request->send(200, "text/plain", "Strip segment " + String(segmentIndex) + " was turned on.");
    });

    _webServer->on("/off", [&](AsyncWebServerRequest *request)
    {
      Serial.println("Received a Off command.");
      int segmentIndex = GetIndex(request);
      _stripState[segmentIndex].onOff = 0;
      xQueueSend(_stripCommandQueue, &_stripState[GetIndex(request)], portMAX_DELAY);

      request->send(200, "text/plain", "Strip segment " + String(segmentIndex) + " was turned off.");
    });

    _webServer->on("/hue", [&](AsyncWebServerRequest *request)
    {
      request->send(200, "text/plain", String(_stripState[GetIndex(request)].hue));
    });

    _webServer->on("/saturation", [&](AsyncWebServerRequest *request)
    {
      request->send(200, "text/plain", String(_stripState[GetIndex(request)].saturation));
    });

    _webServer->on("/brightness", [&](AsyncWebServerRequest *request)
    {
      request->send(200, "text/plain", String(_stripState[GetIndex(request)].brightness));
    });

    _webServer->on("/set", [&](AsyncWebServerRequest *request)
    {      
      if (request->hasParam("h"))
      {
        _stripState[GetIndex(request)].hue = request->getParam("h")->value().toInt();
      }

      if (request->hasParam("s"))
      {
        _stripState[GetIndex(request)].saturation = request->getParam("s")->value().toInt();
      }

      if (request->hasParam("b"))
      {
        _stripState[GetIndex(request)].brightness = request->getParam("b")->value().toInt();
      }      
      
      xQueueSend(_stripCommandQueue, &_stripState[GetIndex(request)], portMAX_DELAY);
      request->send(200);
    });

    _webServer->on("/stripconfig", [&](AsyncWebServerRequest *request)
    {
      if (request->hasParam("length"))
      {
        StripSegment segment;
        if (request->hasParam("index"))
        {
          segment.index = request->getParam("index")->value().toInt();
        }
        else
        {
          segment.index = 0;
        }
        segment.lenght = request->getParam("length")->value().toInt();
        xQueueSend(_stripConfigQueue, &segment, portMAX_DELAY);
        request->send(200, "text/plain", "Segment(" + String(segment.index) + ") was set to " + String(segment.lenght) + " of lenght.");
      }
      else
      {
        request->send(400);
      }
    });

    _webServer->begin();
  }

  unsigned int GetIndex(AsyncWebServerRequest *request)
  {
    if (request->hasParam("index"))
    {
      return request->getParam("index")->value().toInt();
    }
    else
    {
      return 0;
    }
  }

protected:
  AsyncWebServer *_webServer;
  unsigned int _id;
  QueueHandle_t _stripCommandQueue;
  QueueHandle_t _stripConfigQueue;
  unsigned int _onOffState = 0;
  StripSegmentState _stripState[10];
};
