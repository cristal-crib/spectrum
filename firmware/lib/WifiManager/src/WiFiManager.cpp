/**************************************************************
   WiFiManager is a library for the ESP32/ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#include <array>
#include <functional>
#include "WiFiManager.h"

#ifdef ESP32
#include <EEPROM.h>
#endif

#ifdef ESP32_WIFI_AUTOCONNECT_BUG
#include <EEPROM_variables.h>
#endif

namespace
{

const char HTML_HEAD[] PROGMEM =
#include "inc/html_header.html"

    const char CSS_STYLE[] PROGMEM =
#include "inc/glmt.css"

        const char SCRIPT[] PROGMEM =
#include "inc/glmt.js"

            const char HTML_HEAD_END[] PROGMEM = "</head><body>";

const char HTML_ROOT_PAGE_HEADER[] PROGMEM =
#include "inc/html_root_page_header.html"

    const char HTML_CHILD_PAGE_HEADER[] PROGMEM =
#include "inc/html_child_page_header.html"

        const char HTML_MENU_BEGIN[] PROGMEM =
#include "inc/html_menu_begin.html"

            const char HTML_MENU_END[] PROGMEM =
#include "inc/html_menu_end.html"

                const char HTML_MENU_ITEM[] PROGMEM =
#include "inc/html_menu_item.html"

                    const char HTML_ROOT_PAGE_MENU_ITEMS[] PROGMEM =
#include "inc/html_root_menu_items.html"

                        const char HTML_WIFI_FORM_START[] PROGMEM =
#include "inc/html_wifi_from_start.html"

                            const char HTML_WIFI_FORM_PARAM[] PROGMEM =
#include "inc/html_wifi_from_param.html"

                                const char HTML_WIFI_FORM_END[] PROGMEM =
#include "inc/html_wifi_from_end.html"

                                    const char HTML_WIFI_MENU_SCAN[] PROGMEM =
#include "inc/html_wifi_menu_scan_item.html"

                                        const char HTML_END[] PROGMEM =
#include "inc/html_end.html"

                                            const char HTML_INFO[] PROGMEM =
#include "inc/html_info.html"

} // namespace

using namespace wifiman_const;

WiFiManagerParameter::WiFiManagerParameter(const char *custom)
{
  _id = NULL;
  _placeholder = NULL;
  _value = "";

  _customHTML = custom;
}

WiFiManagerParameter::WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length)
{
  init(id, placeholder, defaultValue, length, "");
}

WiFiManagerParameter::WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom)
{
  init(id, placeholder, defaultValue, length, custom);
}

void WiFiManagerParameter::init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom)
{
  _id = id;
  _placeholder = placeholder;
  if (defaultValue != NULL)
  {
    _value = defaultValue;
  }
  _customHTML = custom;
}

WiFiManagerParameter::~WiFiManagerParameter()
{
}

const char *WiFiManagerParameter::getValue()
{
  return _value.c_str();
}
const char *WiFiManagerParameter::getID()
{
  return _id;
}
const char *WiFiManagerParameter::getPlaceholder()
{
  return _placeholder;
}
int WiFiManagerParameter::getValueLength()
{
  return _value.length();
}
const char *WiFiManagerParameter::getCustomHTML()
{
  return _customHTML;
}

WiFiManager::WiFiManager()
{
}

WiFiManager::~WiFiManager()
{
  _params.clear();
}

const String WiFiManager::get_html_head()
{
  return FPSTR(HTML_HEAD);
}

const String WiFiManager::get_css_style()
{
  return FPSTR(CSS_STYLE);
}

const String WiFiManager::get_script()
{
  return FPSTR(SCRIPT);
}

const String WiFiManager::get_html_head_end()
{
  return FPSTR(HTML_HEAD_END);
}

const String WiFiManager::get_html_root_page_header()
{
  return FPSTR(HTML_ROOT_PAGE_HEADER);
}

const String WiFiManager::get_html_child_page_header()
{
  return FPSTR(HTML_CHILD_PAGE_HEADER);
}

const String WiFiManager::get_html_menu_begin()
{
  return FPSTR(HTML_MENU_BEGIN);
}

const String WiFiManager::get_html_menu_end()
{
  return FPSTR(HTML_MENU_END);
}

const String WiFiManager::get_html_menu_item()
{
  return FPSTR(HTML_MENU_ITEM);
}

const String WiFiManager::get_html_root_page_menu_items()
{
  return FPSTR(HTML_ROOT_PAGE_MENU_ITEMS);
}

const String WiFiManager::get_html_root_page_custom_menu_items()
{
  return "";
}

const String WiFiManager::get_html_wifi_form_start()
{
  return FPSTR(HTML_WIFI_FORM_START);
}

const String WiFiManager::get_html_wifi_form_param()
{
  return FPSTR(HTML_WIFI_FORM_PARAM);
}

const String WiFiManager::get_html_wifi_form_end()
{
  return FPSTR(HTML_WIFI_FORM_END);
}

const String WiFiManager::get_html_wifi_menu_scan()
{
  return FPSTR(HTML_WIFI_MENU_SCAN);
}

const String WiFiManager::get_html_end()
{
  return FPSTR(HTML_END);
}

const String WiFiManager::get_html_info()
{
  return FPSTR(HTML_INFO);
}

void WiFiManager::addParameter(WiFiManagerParameter *p)
{
  _params.push_back(p);
  DEBUG_WM("Adding parameter");
  DEBUG_WM(p->getID());
}

void WiFiManager::setupConfigPortal()
{
  DEBUG_WM(F(__PRETTY_FUNCTION__));

#ifdef ESP8266
  dnsServer.reset(new DNSServer());
  server.reset(new ESP8266WebServer(80));
#endif
#ifdef ESP32
  dnsServer.reset(new DNSServer());
  server.reset(new WebServer(80));
#endif

  _configPortalStart = millis();

  DEBUG_WM(F("Configuring access point... "));
  DEBUG_WM(_apName);
  if (_apPassword != NULL)
  {
    if (strlen(_apPassword) < 8 || strlen(_apPassword) > 63)
    {
      // fail passphrase to short or long!
      DEBUG_WM(F("Invalid AccessPoint password. Ignoring"));
      _apPassword = NULL;
    }
    DEBUG_WM(_apPassword);
  }

  //optional soft ip config
  if (_ap_static_ip)
  {
    DEBUG_WM(F("Custom AP IP/GW/Subnet"));
    WiFi.softAPConfig(_ap_static_ip, _ap_static_gw, _ap_static_sn);
  }

  if (_apPassword != NULL)
  {
    WiFi.softAP(_apName, _apPassword); //password option
  }
  else
  {
    WiFi.softAP(_apName);
  }

  delay(500); // Without delay I've seen the IP address blank
  DEBUG_WM(F("AP IP address: "));
  DEBUG_WM(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
  bindHandler();
  server->begin(); // Web server start
  DEBUG_WM(F("HTTP server started"));

// For some reason ESP32 does not autoconnect
#ifdef ESP32_WIFI_AUTOCONNECT_BUG
  auto eeprom = EEPROM_variables::get_var_ptr();
  DEBUG_WM(F("autoconnect ssid=%s, pw=%s"), eeprom->ssid, eeprom->password);
  if (connectWifi(eeprom->ssid, eeprom->password) == WL_CONNECTED)
  {
    DEBUG_WM(F("Connected - IP Address: %i"), WiFi.localIP());
  }
  else
  {
    DEBUG_WM(F("Failed to auto connect: - %d"), WiFi.status());
  }
#endif
}

void WiFiManager::bindHandler()
{
  DEBUG_WM(F(__PRETTY_FUNCTION__));

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server->on("/", std::bind(&WiFiManager::handleRoot, this));
  server->on("/wifi", std::bind(&WiFiManager::handleWifi, this, true));
  server->on("/0wifi", std::bind(&WiFiManager::handleWifi, this, false));
  server->on("/wifisave", std::bind(&WiFiManager::handleWifiSave, this));
  server->on("/i", std::bind(&WiFiManager::handleInfo, this));
  server->on("/r", std::bind(&WiFiManager::handleReset, this, false));
  server->on("/rst", std::bind(&WiFiManager::handleReset, this, true));
  //server->on("/generate_204", std::bind(&WiFiManager::handle204, this));  //Android/Chrome OS captive portal check.
  server->on("/fwlink", std::bind(&WiFiManager::handleRoot, this)); //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server->on("/glmt.css", std::bind(&WiFiManager::handleCssStyle, this));
  server->on("/glmt.js", std::bind(&WiFiManager::handleScript, this));

  server->onNotFound(std::bind(&WiFiManager::handleNotFound, this));
}

boolean WiFiManager::autoConnect()
{
  DEBUG_WM(F(__PRETTY_FUNCTION__));

  String ssid = "ESP" + String(ESP_getChipId());
  return autoConnect(ssid.c_str(), NULL);
}

boolean WiFiManager::autoConnect(String hostname, char const *apName, char const *apPassword)
{
  DEBUG_WM(F(__PRETTY_FUNCTION__));

  DEBUG_WM(F("AutoConnect"));

  // read eeprom for ssid and pass
  //String ssid = getSSID();
  //String pass = getPassword();

  // attempt to connect; should it fail, fall back to AP
  WiFi.mode(WIFI_STA);

  if (connectWifi("", "", hostname) == WL_CONNECTED)
  {
    DEBUG_WM(F("IP Address:"));
    DEBUG_WM(WiFi.localIP());
    //connected
    return true;
  }

  return startConfigPortal(apName, apPassword);
}

boolean WiFiManager::configPortalHasTimeout()
{
#ifdef ESP8266
  if (_configPortalTimeout == 0 || wifi_softap_get_station_num() > 0)
  {
#endif
#ifdef ESP32
    if (_configPortalTimeout == 0)
    { // TODO
#endif
      _configPortalStart = millis(); // kludge, bump configportal start time to skew timeouts
      return false;
    }
    return (millis() > _configPortalStart + _configPortalTimeout);
  }

  boolean WiFiManager::startConfigPortal()
  {
    DEBUG_WM(F(__PRETTY_FUNCTION__));

    String ssid = "ESP" + String(ESP_getChipId());
    return startConfigPortal(ssid.c_str(), NULL);
  }

  boolean WiFiManager::startConfigPortal(String hostname, char const *apName, char const *apPassword)
  {
    DEBUG_WM(F(__PRETTY_FUNCTION__));

    //setup AP
    WiFi.mode(WIFI_AP_STA);
    DEBUG_WM("SET AP STA");

    _apName = apName;
    _apPassword = apPassword;

    //notify we entered AP mode
    if (_apcallback != NULL)
    {
      _apcallback(this);
    }

    connect = false;
    setupConfigPortal();

    while (1)
    {

      // check if timeout
      if (configPortalHasTimeout())
        break;

      //DNS
      dnsServer->processNextRequest();
      //HTTP
      server->handleClient();

      if (connect)
      {
        connect = false;
        delay(2000);
        DEBUG_WM(F("Connecting to new AP"));

        // using user-provided  _ssid, _pass in place of system-stored ssid and pass
        if (connectWifi(_ssid, _pass, hostname) != WL_CONNECTED)
        {
          DEBUG_WM(F("Failed to connect."));
        }
        else
        {
          //connected
          WiFi.mode(WIFI_STA);
          //notify that configuration has changed and any optional parameters should be saved
          if (_savecallback != NULL)
          {
            //todo: check if any custom parameters actually exist, and check if they really changed maybe
            _savecallback();
          }
          break;
        }

        if (_shouldBreakAfterConfig)
        {
          //flag set to exit after config after trying to connect
          //notify that configuration has changed and any optional parameters should be saved
          if (_savecallback != NULL)
          {
            //todo: check if any custom parameters actually exist, and check if they really changed maybe
            _savecallback();
          }
          break;
        }
      }
      yield();
    }

    server.reset();
    dnsServer.reset();

    return WiFi.status() == WL_CONNECTED;
  }

  void WiFiManager::startConfigPortalMulti(String hostname, char const *apName, char const *apPassword)
  {
    DEBUG_WM(F(__PRETTY_FUNCTION__));

    //setup AP
    WiFi.mode(WIFI_AP_STA);
    DEBUG_WM("SET AP STA");

    WiFi.disconnect();

    // Keep entities as it disappears
    static String apName_entity = apName;
    static String apPassword_entity = apPassword;

    _apName = apName_entity.c_str();
    _apPassword = apPassword_entity.c_str();

    //notify we entered AP mode
    if (_apcallback != NULL)
    {
      _apcallback(this);
    }

    connect = false;
    setupConfigPortal();
  }

  void WiFiManager::updateConfigPortalMulti()
  {
    do
    {
      //DNS
      dnsServer->processNextRequest();
      //HTTP
      server->handleClient();

      if (connect)
      {
        connect = false;
        delay(2000);
        DEBUG_WM(F("Connecting to new AP"));

        // using user-provided  _ssid, _pass in place of system-stored ssid and pass
        if (connectWifi(_ssid, _pass, "") != WL_CONNECTED)
        {
          DEBUG_WM(F("Failed to connect."));
        }
        else
        {
          //connected
          //notify that configuration has changed and any optional parameters should be saved
          DEBUG_WM(F("Connected"));
          if (_savecallback != NULL)
          {
            //todo: check if any custom parameters actually exist, and check if they really changed maybe
            _savecallback();
          }
          break;
        }

        if (_shouldBreakAfterConfig)
        {
          //flag set to exit after config after trying to connect
          //notify that configuration has changed and any optional parameters should be saved
          if (_savecallback != NULL)
          {
            //todo: check if any custom parameters actually exist, and check if they really changed maybe
            _savecallback();
          }
          break;
        }
      }
    } while (0);
  }

  int WiFiManager::connectWifi(String ssid, String pass, String hostname)
  {
    DEBUG_WM(F("Connecting as wifi client..."));
    DEBUG_WM(F("id:<%s> passwd:<%s>"), ssid.c_str(), pass.c_str());

    // check if we've got static_ip settings, if we do, use those.
    if (_sta_static_ip)
    {
      DEBUG_WM(F("Custom STA IP/GW/Subnet"));
      WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
      DEBUG_WM(WiFi.localIP());
    }
    //fix for auto connect racing issue
    if (WiFi.status() == WL_CONNECTED)
    {
      DEBUG_WM("Already connected. Bailing out.");
      return WL_CONNECTED;
    }
    //check if we have ssid and pass and force those, if not, try with last saved values
    if (ssid != "")
    {
      WiFi.begin(ssid.c_str(), pass.c_str());
      if (hostname != "")
      {
        WiFi.setHostname(hostname.c_str());
      }
    }
    else
    {
      if (WiFi.SSID())
      {
        DEBUG_WM("Using last saved values, should be faster");
#ifdef ESP8266
        //trying to fix connection in progress hanging
        ETS_UART_INTR_DISABLE();
        wifi_station_disconnect();
        ETS_UART_INTR_ENABLE();
#endif
#ifdef ESP32
        esp_wifi_disconnect();
#endif
        WiFi.begin();
        if (hostname != "")
        {
          WiFi.setHostname(hostname.c_str());
        }
      }
      else
      {
        DEBUG_WM("No saved credentials");
      }
    }

    int connRes = waitForConnectResult();
    DEBUG_WM("Connection result: ");
    DEBUG_WM(connRes);
    //not connected, WPS enabled, no pass - first attempt
    if (_tryWPS && connRes != WL_CONNECTED && pass == "")
    {
      startWPS();
      //should be connected at the end of WPS
      connRes = waitForConnectResult();
    }
    return connRes;
  }

  uint8_t WiFiManager::waitForConnectResult()
  {
    DEBUG_WM(F(__PRETTY_FUNCTION__));

    if (_connectTimeout == 0)
    {
      return WiFi.waitForConnectResult();
    }
    else
    {
      DEBUG_WM(F("Waiting for connection result with time out"));
      unsigned long start = millis();
      boolean keepConnecting = true;
      uint8_t status;
      while (keepConnecting)
      {
        status = WiFi.status();
        if (millis() > start + _connectTimeout)
        {
          keepConnecting = false;
          DEBUG_WM(F("Connection timed out"));
        }
        if (status == WL_CONNECTED || status == WL_CONNECT_FAILED)
        {
          keepConnecting = false;
        }
        delay(100);
      }
      return status;
    }
  }

  void WiFiManager::startWPS()
  {
    DEBUG_WM(F(__PRETTY_FUNCTION__));

#ifdef ESP8266
    DEBUG_WM("START WPS");
    WiFi.beginWPSConfig();
    DEBUG_WM("END WPS");
#endif
#ifdef ESP32
    // TODO
    DEBUG_WM("ESP32 WPS TODO");
#endif
  }
  /*
  String WiFiManager::getSSID() {
  if (_ssid == "") {
    DEBUG_WM(F("Reading SSID"));
    _ssid = WiFi.SSID();
    DEBUG_WM(F("SSID: "));
    DEBUG_WM(_ssid);
  }
  return _ssid;
  }

  String WiFiManager::getPassword() {
  if (_pass == "") {
    DEBUG_WM(F("Reading Password"));
    _pass = WiFi.psk();
    DEBUG_WM("Password: " + _pass);
    //DEBUG_WM(_pass);
  }
  return _pass;
  }
*/
  String WiFiManager::getConfigPortalSSID()
  {
    return _apName;
  }

  void WiFiManager::resetSettings()
  {
    DEBUG_WM(F("settings invalidated"));
    DEBUG_WM(F("THIS MAY CAUSE AP NOT TO START UP PROPERLY. YOU NEED TO COMMENT IT OUT AFTER ERASING THE DATA."));
    WiFi.disconnect(true);
    //delay(200);
  }
  void WiFiManager::setTimeout(unsigned long seconds)
  {
    setConfigPortalTimeout(seconds);
  }

  void WiFiManager::setConfigPortalTimeout(unsigned long seconds)
  {
    _configPortalTimeout = seconds * 1000;
  }

  void WiFiManager::setConnectTimeout(unsigned long seconds)
  {
    _connectTimeout = seconds * 1000;
  }

  void WiFiManager::setDebugOutput(boolean debug)
  {
    _debug = debug;
  }

  void WiFiManager::setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn)
  {
    _ap_static_ip = ip;
    _ap_static_gw = gw;
    _ap_static_sn = sn;
  }

  void WiFiManager::setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn)
  {
    setUseStaticIP(true);
    _sta_static_ip = ip;
    _sta_static_gw = gw;
    _sta_static_sn = sn;
  }

  void WiFiManager::setMinimumSignalQuality(int quality)
  {
    _minimumQuality = quality;
  }

  void WiFiManager::setBreakAfterConfig(boolean shouldBreak)
  {
    _shouldBreakAfterConfig = shouldBreak;
  }

  void WiFiManager::set_current_value(String & str)
  {
    str.replace(FPSTR(FS_T), _apName);

    String wifi_conn = (WiFi.status() == WL_CONNECTED) ? F("On") : F("Off");
    str.replace(F("{conn}"), wifi_conn);

    if (str.indexOf(FPSTR(FS_AIP)) >= 0)
    {
      String ip_adr = WiFi.softAPIP().toString();
      str.replace(FPSTR(FS_AIP), ip_adr);
    }

    if (str.indexOf(FPSTR(FS_IPT)) >= 0)
    {
      String ip_adr;
      if (WiFi.status() == WL_CONNECTED)
      {
        ip_adr = WiFi.localIP().toString();
      }
      else
      {
        ip_adr = F("None");
      }
      str.replace(FPSTR(FS_IPT), ip_adr);
    }
  }

  void WiFiManager::handleCssStyle()
  {
    DEBUG_WM(F("handleCssStyle()"));
    String page = get_css_style();
    server->sendHeader(FPSTR(FS_CONTENT_LENGTH), String(page.length()));
    server->send(200, F("text/css"), page);
  }

  void WiFiManager::handleScript()
  {
    DEBUG_WM(F("handleScript()"));
    String page = get_script();
    server->sendHeader(FPSTR(FS_CONTENT_LENGTH), String(page.length()));
    server->send(200, F("text/javascript"), page);
  }

  /** Handle root or redirect to captive portal */
  void WiFiManager::handleRoot()
  {
    DEBUG_WM(F("Handle root"));
    if (captivePortal())
    { // If caprive portal redirect instead of displaying the page.
      return;
    }

    String page = get_html_head();
    //  page.replace(FPSTR(FS_V), "Options");
    //  page += get_script();
    //  page += get_css_style();
    page += _customHeadElement;
    page += get_html_head_end();

    page += get_html_root_page_header();
    page += get_html_menu_begin();
    page += get_html_root_page_menu_items();
    page += get_html_menu_end();
    page += get_html_end();

    {
      String item = get_html_root_page_custom_menu_items();
      page.replace(F("{co}"), item);
    }

    set_current_value(page);

    server->sendHeader(FPSTR(FS_CONTENT_LENGTH), String(page.length()));
    server->send(200, FPSTR(FS_TEXT_HTML), page);
  }

  /** Wifi config page handler */
  void WiFiManager::handleWifi(boolean scan)
  {

    String page = get_html_head();
    page += _customHeadElement;
    page += get_html_head_end();

    page += get_html_child_page_header();
    page.replace(FPSTR(FS_TT), F("WI-FI"));
    page.replace(FPSTR(FS_HR), F("/wifi"));
    page.replace(FPSTR(FS_SB), F("♻"));
    page += get_html_menu_begin();

    if (scan)
    {
      int n = WiFi.scanNetworks();
      DEBUG_WM(F("Scan done"));
      if (n == 0)
      {
        DEBUG_WM(F("No networks found"));
        page += F("No networks found. Refresh to scan again.");
      }
      else
      {

        //sort networks
        int indices[n];
        for (int i = 0; i < n; i++)
        {
          indices[i] = i;
        }

        // RSSI SORT

        // old sort
        for (int i = 0; i < n; i++)
        {
          for (int j = i + 1; j < n; j++)
          {
            if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
            {
              std::swap(indices[i], indices[j]);
            }
          }
        }

        /*std::sort(indices, indices + n, [](const int & a, const int & b) -> bool
        {
        return WiFi.RSSI(a) > WiFi.RSSI(b);
        });*/

        // remove duplicates ( must be RSSI sorted )
        if (_removeDuplicateAPs)
        {
          String cssid;
          for (int i = 0; i < n; i++)
          {
            if (indices[i] == -1)
              continue;
            cssid = WiFi.SSID(indices[i]);
            for (int j = i + 1; j < n; j++)
            {
              if (cssid == WiFi.SSID(indices[j]))
              {
                DEBUG_WM("DUP AP: " + WiFi.SSID(indices[j]));
                indices[j] = -1; // set dup aps to index -1
              }
            }
          }
        }

        //display networks in page
        for (int i = 0; i < n; i++)
        {
          if (indices[i] == -1)
            continue; // skip dups
          DEBUG_WM(WiFi.SSID(indices[i]));
          DEBUG_WM(WiFi.RSSI(indices[i]));
          int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));

          if (_minimumQuality == -1 || _minimumQuality < quality)
          {
            String item = get_html_wifi_menu_scan();
            String rssiQ;
            rssiQ += quality;
            item.replace(FPSTR(FS_V), WiFi.SSID(indices[i]));
            item.replace(F("{r}"), rssiQ);
#ifdef ESP8266
            if (WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE)
            {
#endif
#ifdef ESP32
              if (WiFi.encryptionType(indices[i]) != WIFI_AUTH_OPEN)
              {
#endif
                item.replace(FPSTR(FS_I), F("l"));
              }
              else
              {
                item.replace(FPSTR(FS_I), "");
              }
              page += item;
              delay(0);
            }
            else
            {
              DEBUG_WM(F("Skipping due to quality"));
            }
          }
        }
      }
      page += get_html_menu_end();

      page += get_html_wifi_form_start();
      // add the extra parameters to the form
      for (auto param : _params)
      {
        String pitem = get_html_wifi_form_param();
        if (param->getID() != NULL)
        {
          pitem.replace(FPSTR(FS_I), param->getID());
          pitem.replace(FPSTR(FS_N), param->getID());
          pitem.replace(FPSTR(FS_N), param->getPlaceholder());
          pitem.replace(FPSTR(FS_L), String(param->getValueLength()));
          pitem.replace(FPSTR(FS_V), param->getValue());
          pitem.replace(FPSTR(FS_C), param->getCustomHTML());
        }
        else
        {
          pitem = param->getCustomHTML();
        }

        page += pitem;
      }

      if (isUseStaticIP())
      {
        String item = get_html_wifi_form_param();
        item.replace(FPSTR(FS_I), FPSTR(FS_IP));
        item.replace(FPSTR(FS_N), FPSTR(FS_IP));
        item.replace(FPSTR(FS_N), F("Static IP"));
        item.replace(FPSTR(FS_L), FPSTR(FS_15));
        if (_sta_static_ip)
        {
          item.replace(FPSTR(FS_V), _sta_static_ip.toString());
        }
        else
        {
          item.replace(FPSTR(FS_V), "");
        }

        page += item;

        item = get_html_wifi_form_param();
        item.replace(FPSTR(FS_I), FPSTR(FS_GW));
        item.replace(FPSTR(FS_N), FPSTR(FS_GW));
        item.replace(FPSTR(FS_N), F("Static Gateway"));
        item.replace(FPSTR(FS_L), FPSTR(FS_15));
        if (_sta_static_gw)
        {
          item.replace(FPSTR(FS_V), _sta_static_gw.toString());
        }
        else
        {
          item.replace(FPSTR(FS_V), "");
        }

        page += item;

        item = get_html_wifi_form_param();
        item.replace(FPSTR(FS_I), FPSTR(FS_SN));
        item.replace(FPSTR(FS_N), FPSTR(FS_SN));
        item.replace(FPSTR(FS_N), F("Subnet"));
        item.replace(FPSTR(FS_L), FPSTR(FS_15));
        if (_sta_static_sn)
        {
          item.replace(FPSTR(FS_V), _sta_static_sn.toString());
        }
        else
        {
          item.replace(FS_V, "");
        }

        page += item;
      }
      page += get_html_wifi_form_end();

      page += get_html_end();
      set_current_value(page);

      server->sendHeader(FPSTR(FS_CONTENT_LENGTH), String(page.length()));
      server->send(200, FPSTR(FS_TEXT_HTML), page);

      DEBUG_WM(F("Sent config page"));
    }

    /** Handle the WLAN save form and redirect to WLAN config page again */
    void WiFiManager::handleWifiSave()
    {
      DEBUG_WM(F("WiFi save"));

      //SAVE/connect here
      _ssid = server->arg(F("s")).c_str();
      _pass = server->arg(F("p")).c_str();

// For some reason ESP32 does not autoconnect
#ifdef ESP32_WIFI_AUTOCONNECT_BUG
      {
        auto eeprom = EEPROM_variables::get_var_ptr();
        memset(eeprom->ssid, 0, sizeof(eeprom->ssid));
        memset(eeprom->password, 0, sizeof(eeprom->password));
        strncpy(eeprom->ssid, _ssid.c_str(), sizeof(eeprom->ssid) - 1);
        strncpy(eeprom->password, _pass.c_str(), sizeof(eeprom->password) - 1);
        EEPROM_variables::commit();
      }
#endif

      //parameters
      for (auto param : _params)
      {
        //read parameter
        String value = server->arg(param->getID()).c_str();
        //store
        param->_value = value;
        DEBUG_WM(F("Parameter"));
        DEBUG_WM(param->getID());
        DEBUG_WM(value);
      }

      if (server->arg(FPSTR(FS_IP)) != "")
      {
        DEBUG_WM(F("static ip"));
        DEBUG_WM(server->arg(FPSTR(FS_IP)));
        //_sta_static_ip.fromString(server->arg(FPSTR(FS_IP)));
        String ip = server->arg(FPSTR(FS_IP));
        optionalIPFromString(&_sta_static_ip, ip.c_str());
      }
      if (server->arg(FPSTR(FS_GW)) != "")
      {
        DEBUG_WM(F("static gateway"));
        DEBUG_WM(server->arg(FPSTR(FS_GW)));
        String gw = server->arg(FPSTR(FS_GW));
        optionalIPFromString(&_sta_static_gw, gw.c_str());
      }
      if (server->arg(FPSTR(FS_SN)) != "")
      {
        DEBUG_WM(F("static netmask"));
        DEBUG_WM(server->arg(FPSTR(FS_SN)));
        String sn = server->arg(FPSTR(FS_SN));
        optionalIPFromString(&_sta_static_sn, sn.c_str());
      }

      //  String page = FPSTR(HTTP_HEAD);
      //  page.replace(FS_V, "Credentials Saved");
      //  page += get_http_script();
      //  page += get_http_script();
      //  page += _customHeadElement;
      //  page += get_http_head_end();
      //  page += FPSTR(HTTP_SAVED);
      //  page += get_http_end();
      //
      //  server->sendHeader(FPSTR(FS_CONTENT_LENGTH), String(page.length()));
      //  server->send(200, FPSTR(FS_TEXT_HTML), page);

      server->sendHeader(FPSTR(FS_LOCATION), F("/"), true);
      server->send(303, FPSTR(FS_TEXT_PLAIN), "");

      DEBUG_WM(F("Sent wifi save page"));

      connect = true; //signal ready to connect/reset
    }

    /** Handle the info page */
    void WiFiManager::handleInfo()
    {
      DEBUG_WM(F("Info"));

      String page = get_html_head();
      page += _customHeadElement;
      page += get_html_head_end();

      page += get_html_child_page_header();
      page.replace(FPSTR(FS_T), FPSTR(FS_DEVICE_INFORMATION));
      page.replace(FPSTR(FS_TT), FPSTR(FS_DEVICE_INFORMATION));

      page += get_html_info();
      // chip id
      page.replace(F("{cid}"), String(ESP_getChipId()));
#ifdef ESP8266
      // flash chip id
      page.replace(F("{fid}"), String(ESP.getFlashChipId()));
#endif
      // IDE Flash Size
      page.replace(F("{fsz}"), String(ESP.getFlashChipSize()));
#ifdef ESP8266
      // Real Flash Size
      page.replace(F("{rfz}"), String(ESP.getFlashChipRealSize()));
#endif
      // Soft AP IP
      page.replace(F("{sip}"), WiFi.softAPIP().toString());
      // Soft AP MAC
      page.replace(F("{smc}"), WiFi.softAPmacAddress());
      // Soft Station IP
      page.replace(F("{tip}"), WiFi.localIP().toString());
      // Soft Station Gateway
      page.replace(F("{tgw}"), WiFi.gatewayIP().toString());
      // Soft Station Subnetmask
      page.replace(F("{tmk}"), WiFi.subnetMask().toString());
      // Soft Station DNS
      page.replace(F("{tns}"), WiFi.dnsIP().toString());
      // Station MAC
      page.replace(F("{tmc}"), WiFi.macAddress());
      // Free Heap
      page.replace(F("{frh}"), String(ESP.getFreeHeap()));
#ifdef ESP8266
      // Sketch Size
      page.replace(F("{skz}"), String(ESP.getSketchSize()));
      // Sketch MD5
      page.replace(F("{md5}"), String(ESP.getSketchMD5()));
      // Free Sketch Space
      page.replace(F("{fss}"), String(ESP.getFreeSketchSpace()));
#endif
      // Flash Chip Mode
      page.replace(F("{fcm}"), String(ESP.getFlashChipMode()));
      // Flash Chip Speed
      page.replace(F("{fcs}"), String(ESP.getFlashChipSpeed()));
#ifdef ESP8266
      // Core Version
      page.replace(F("{cve}"), String(ESP.getCoreVersion()));
#endif
      // SDK Version
      page.replace(F("{sdk}"), String(ESP.getSdkVersion()));
#ifdef ESP8266
      // Boot Version
      page.replace(F("{btv}"), String(ESP.getBootVersion()));
      // Boot Mode
      page.replace(F("{btm}"), String(ESP.getBootMode()));
      // VCC
      page.replace(F("{vcc}"), String(ESP.getVcc()));
#endif
      // CPU Speed
      page.replace(F("{csp}"), String(ESP.getCpuFreqMHz()));

      page += get_html_end();
      server->sendHeader(FPSTR(FS_CONTENT_LENGTH), String(page.length()));
      server->send(200, FPSTR(FS_TEXT_HTML), page);

      DEBUG_WM(F("Sent info page"));
    }

    /** Handle the reset page */
    void WiFiManager::handleReset(bool is_also_setting)
    {
      DEBUG_WM(F("Reset"));

      //  String page = get_html_head();
      //  page.replace(FS_V, "Reset");
      //  page += get_script();
      //  page += get_css_style();
      //  page += _customHeadElement;
      //  page += get_html_head_end();
      //  page += F("Module will reset in a few seconds.");
      //  page += get_html_end();
      //
      //  server->sendHeader(FPSTR(FS_CONTENT_LENGTH), String(page.length()));
      //  server->send(200, FPSTR(FS_TEXT_HTML), page);

      DEBUG_WM(F("Sent reset page"));

      delay(5000);
      if (is_also_setting)
      {
        resetSettings();
#ifdef ESP8266
        ESP.eraseConfig();
#endif

// From ESP 8266
#ifdef ESP32
        {
          const size_t cfgSize = 0x4000;
          size_t cfgAddr = ESP.getFlashChipSize() - cfgSize;

          for (size_t offset = 0; offset < cfgSize; offset += SPI_FLASH_SEC_SIZE)
          {
            if (!ESP.flashEraseSector((cfgAddr + offset) / SPI_FLASH_SEC_SIZE))
            {
              break;
            }
          }
        }
#endif

        delay(5000);
      }

      pinMode(0, OUTPUT);
      digitalWrite(0, 1);
      pinMode(2, OUTPUT);
      digitalWrite(2, 1);
      //  ESP.reset();
      ESP.restart();

      delay(2000);
    }

    void WiFiManager::handleNotFound()
    {
      if (captivePortal())
      { // If captive portal redirect instead of displaying the error page.
        return;
      }
      String message = F("File Not Found\n\n");
      message += F("URI: ");
      message += server->uri();
      message += F("\nMethod: ");
      message += (server->method() == HTTP_GET) ? F("GET") : F("POST");
      message += F("\nArguments: ");
      message += server->args();
      message += F("\n");

      for (uint8_t i = 0; i < server->args(); i++)
      {
        message += F(" ");
        message += server->argName(i) + F(": ") + server->arg(i) + F("\n");
      }
      server->sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
      server->sendHeader(F("Pragma"), F("no-cache"));
      server->sendHeader(F("Expires"), F("-1"));
      server->sendHeader(FPSTR(FS_CONTENT_LENGTH), String(message.length()));
      server->send(404, FPSTR(FS_TEXT_PLAIN), message);
    }

    /** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
    boolean WiFiManager::captivePortal()
    {
      DEBUG_WM(F(__PRETTY_FUNCTION__));

      if (!isIp(server->hostHeader()))
      {
        DEBUG_WM(F("Request redirected to captive portal"));
        server->sendHeader(FPSTR(FS_LOCATION), String(F("http://")) + toStringIp(server->client().localIP()), true);
        server->send(302, FPSTR(FS_TEXT_PLAIN), ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server->client().stop();                     // Stop is needed because we sent no content length
        return true;
      }
      return false;
    }

    //start up config portal callback
    void WiFiManager::setAPCallback(void (*func)(WiFiManager * myWiFiManager))
    {
      _apcallback = func;
    }

    //start up save config callback
    void WiFiManager::setSaveConfigCallback(void (*func)(void))
    {
      _savecallback = func;
    }

    //sets a custom element to add to head, like a new style tag
    void WiFiManager::setCustomHeadElement(const char *element)
    {
      _customHeadElement = element;
    }

    //if this is true, remove duplicated Access Points - defaut true
    void WiFiManager::setRemoveDuplicateAPs(boolean removeDuplicates)
    {
      _removeDuplicateAPs = removeDuplicates;
    }

    int WiFiManager::getRSSIasQuality(int RSSI)
    {
      int quality = 0;

      if (RSSI <= -100)
      {
        quality = 0;
      }
      else if (RSSI >= -50)
      {
        quality = 100;
      }
      else
      {
        quality = 2 * (RSSI + 100);
      }
      return quality;
    }

    /** Is this an IP? */
    boolean WiFiManager::isIp(String str)
    {
      for (int i = 0; i < str.length(); i++)
      {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9'))
        {
          return false;
        }
      }
      return true;
    }

    /** IP to String? */
    String WiFiManager::toStringIp(IPAddress ip)
    {
      String res = "";
      for (int i = 0; i < 3; i++)
      {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
      }
      res += String(((ip >> 8 * 3)) & 0xFF);
      return res;
    }

#if WIFIMAN_DEBUG

    // source
    // https://github.com/nkolban/ESPLibs/blob/master/ArduinoLibs/Common/ESP_Log.cpp

#define Serial1 Serial

#define ARDBUFFER 16 //Buffer for storing intermediate strings. Performance may vary depending on size.

    /**
 * Perform a "printf" to the Arduino Serial port.  The supported
 * printf flags are:
 * - %d - Print an integer
 * - %l - Print a long integer
 * - %c - Print a character
 * - %s - Print a string
 * - %x - Print an integer in hex
 * - %i - Print an IP address
add to:
 * - %S - Print a string(String*)
 * - %p - Print a pointer
 */
    void WiFiManager::ESPLog::ardprintf(const String &str, ...)
    {
      int i, j;
      char temp[ARDBUFFER + 1];

      va_list vargs;
      va_start(vargs, str);
      //Iterate over formatting string
      for (i = 0, j = 0; str[i] != '\0'; i++)
      {
        if (str[i] == '%')
        {
          //Clear buffer
          temp[j] = '\0';
          Serial1.print(temp);
          j = 0;
          temp[0] = '\0';

          //Process argument
          switch (str[++i])
          {
          case 'd':
            Serial1.print(va_arg(vargs, int));
            break;
          case 'l':
            Serial1.print(va_arg(vargs, long));
            break;
          case 'f':
            Serial1.print(va_arg(vargs, double));
            break;
          case 'c':
            Serial1.print((char)va_arg(vargs, int));
            break;
          case 's':
            Serial1.print(va_arg(vargs, char *));
            break;
          case 'x':
            Serial1.print(va_arg(vargs, int), HEX);
            break;
          case 'i':
            char *p;
            p = va_arg(vargs, char *);
            Serial1.print(String((int)p[0]) + "." + String((int)p[1]) + "." + String((int)p[2]) + "." + String((int)p[3]));
            break;
          case 'S':
          {
            auto arg = va_arg(vargs, String *);
            Serial1.print(*arg);
            break;
          }
          case 'p':
            Serial1.print(va_arg(vargs, uint32_t), HEX);
            break;
          default:;
          };
        }
        else
        {
          //Add to buffer
          temp[j] = str[i];
          j = (j + 1) % ARDBUFFER;
          if (j == 0) //If buffer is full, empty buffer.
          {
            temp[ARDBUFFER] = '\0';
            Serial1.print(temp);
            temp[0] = '\0';
          }
        }
      };
      // If there are any output characters not yet sent, send them now.
      if (j != 0)
      {
        temp[j] = '\0';
        Serial1.print(temp);
      }

      Serial1.print("\n");

      //Serial1.println(); //Print trailing newline
      va_end(vargs);
    } // End of ardprintf

    /**
 * Create and dump a hex array of the content of memory.
 */
    void WiFiManager::ESPLog::dumpHex(const char *buf, int size)
    {
      ESPLog::ardprintf("Dump hex of address: 0x%x for %d\n", buf, size);
      int diff = ((int)buf) % 16;
      int lineAddr = (int)buf - diff;

      ESPLog::ardprintf("%x ", (int)lineAddr);
      for (int i = 0; i < diff; i++)
      {
        ESPLog::ardprintf("   ");
      }
      size += diff;
      buf = buf - diff;
      for (int i = diff; i < size; i++)
      {
        if (i > 0 && i % 16 == 0)
        {
          ESPLog::ardprintf("\n");
          lineAddr += 16;
          ESPLog::ardprintf("%x ", (int)lineAddr);
        }
        char c = buf[i];
        if (c <= 0xf)
        {
          ESPLog::ardprintf("0");
        }
        ESPLog::ardprintf("%x ", (int)c);
      }
      ESPLog::ardprintf("\n");
    } // End of dumpHex

    /**
 * Create and dump a hex array of the content of memory.
 */
    void WiFiManager::ESPLog::dumpHex(const char *from, const char *to)
    {
      int size = to - from;
      if (size <= 0)
      {
        ESPLog::ardprintf("ESPLog::dumpHex: Error end  (0x%x) is < start (0x%x)\n", from, to);
        return;
      }
      ESPLog::dumpHex(from, size);
    } // End of dumpHex

#endif // #if WIFIMAN_DEBUG
