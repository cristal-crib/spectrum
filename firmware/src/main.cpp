#include "main.h"

#if BOARD_VERSION == 1
#define LED_STRIP_PIN 22
#elif BOARD_VERSION == 2 || BOARD_VERSION == 4
#define LED_STRIP_PIN 17
#endif

StatusLedDriver statusLedDriver;

void setup()
{
    Serial.begin(115200);
    InitializeBasicDrivers();

    statusLedDriver.SetLedState(Solid, 255, 0, 255);
}

void loop()
{
    delay(1000);
}


void InitializeBasicDrivers()
{
    statusLedDriver.Init();
}

void SetBoardState(BoardState boardState)
{
    switch (boardState)
    {
    case SeekingWifi:
        statusLedDriver.SetLedState(Breathe, 255, 0, 0);
    case WifiConnected:
        statusLedDriver.SetLedState(Breathe, 0, 255, 0);
    case Ready:
        statusLedDriver.SetLedState(Solid, 0, 5, 0);
    }
}
