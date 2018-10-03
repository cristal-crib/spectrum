#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <LedStripDriver.h>
#include <WebDriver.h>
#include <PairButtonDriver.h>
#include <StatusLedDriver.h>
#include "LedStripManager.h"

enum BoardState
{
    SeekingWifi,
    WifiConnected,
    Ready
};

void InitializeCommandQueues();
void InitializeBasicDrivers();
void InitializeDrivers();
void InitializeWifi();
void SetBoardState(BoardState boardState);
