#include "main.h"

#if BOARD_VERSION == 1
#define LED_STRIP_PIN 22
#elif BOARD_VERSION == 2
#define LED_STRIP_PIN 17
#endif

LedStripDriver ledStripDriver(LED_STRIP_PIN);
WebDriver webDriver;
StatusLedDriver statusLedDriver;
LedStripManager ledStripManager;
QueueHandle_t stripCommandQueue;
QueueHandle_t stripConfigQueue;

void setup()
{
    Serial.begin(115200);
    InitializeCommandQueues();
    InitializeBasicDrivers();

    SetBoardState(SeekingWifi);
    InitializeWifi();
    SetBoardState(WifiConnected);
    InitializeDrivers();
    SetBoardState(Ready);
}

void loop()
{
}

void InitializeCommandQueues()
{
    stripCommandQueue = xQueueCreate(1, sizeof(StripSegmentState));
    stripConfigQueue = xQueueCreate(1, sizeof(StripSegment));
}

void InitializeBasicDrivers()
{
    statusLedDriver.Init();
}

void InitializeDrivers()
{
    ledStripManager.Init(stripCommandQueue, stripConfigQueue, &ledStripDriver);
    webDriver.Init(stripCommandQueue, stripConfigQueue);
}

void InitializeWifi()
{
    WiFiManager wifiManager;
    String ssid = "LED Controller " + String(ESP_getChipId());
    wifiManager.autoConnect("cristalcrib-strip-ctrl-" + String(ESP_getChipId()), ssid.c_str());
    Serial.println("Connected to WiFi SSID: " + WiFi.SSID());
    Serial.println("Hostname: " + String(WiFi.getHostname()));
    Serial.println("IP: " + WiFi.localIP().toString());
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
