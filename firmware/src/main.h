#pragma once

#include <Arduino.h>
#include <PairButtonDriver.h>
#include <StatusLedDriver.h>

enum BoardState
{
    SeekingWifi,
    WifiConnected,
    Ready
};

void InitializeCommandQueues();
void InitializeBasicDrivers();
void SetBoardState(BoardState boardState);
