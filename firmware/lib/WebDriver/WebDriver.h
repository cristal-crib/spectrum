#pragma once

#include <WebServer.h>
#include <string>
#include "ESPAsyncWebServer.h"

struct StripState
{
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
    _stripState.onOff = 0;
    _stripState.brightness = 0;
    _stripState.hue = 0;
    _stripState.saturation = 100;

    _stripCommandQueue = stripCommandQueue;
    _stripConfigQueue = stripConfigQueue;
    _webServer = new AsyncWebServer(80);

    _webServer->on("/status", [&](AsyncWebServerRequest *request)
    {
      request->send(200, "text/plain", String(_stripState.onOff));
    });

    _webServer->on("/on", [&](AsyncWebServerRequest *request)
    {
      Serial.println("Received a On command.");
      _stripState.onOff = 1;
      xQueueSend(_stripCommandQueue, &_stripState, portMAX_DELAY);
      request->send(200, "text/plain", "Strip was turned on.");
    });

    _webServer->on("/off", [&](AsyncWebServerRequest *request)
    {
      Serial.println("Received a Off command.");
      _stripState.onOff = 0;
      xQueueSend(_stripCommandQueue, &_stripState, portMAX_DELAY);
      request->send(200, "text/plain", "Strip was turned off.");
    });

    _webServer->on("/hue", [&](AsyncWebServerRequest *request)
    {
      request->send(200, "text/plain", String(_stripState.hue));
    });

    _webServer->on("/saturation", [&](AsyncWebServerRequest *request)
    {
      request->send(200, "text/plain", String(_stripState.saturation));
    });

    _webServer->on("/brightness", [&](AsyncWebServerRequest *request)
    {
      request->send(200, "text/plain", String(_stripState.brightness));
    });

    _webServer->on("/set", [&](AsyncWebServerRequest *request)
    {      
      if (request->hasParam("h"))
      {
        _stripState.hue = request->getParam("h")->value().toInt();
      }

      if (request->hasParam("s"))
      {
        _stripState.saturation = request->getParam("s")->value().toInt();
      }

      if (request->hasParam("b"))
      {
        _stripState.brightness = request->getParam("b")->value().toInt();
      }      
      
      xQueueSend(_stripCommandQueue, &_stripState, portMAX_DELAY);
      request->send(200);
    });

    _webServer->on("/stripConfig", [&](AsyncWebServerRequest *request)
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

protected:
  AsyncWebServer *_webServer;
  unsigned int _id;
  QueueHandle_t _stripCommandQueue;
  QueueHandle_t _stripConfigQueue;
  unsigned int _onOffState = 0;
  StripState _stripState;
};
