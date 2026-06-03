// Stubs for GPIO, ScreenManager, and other excluded modules.
// Web server functions are now in DFWebServer.cpp (ESPAsyncWebServer-based).
#include <Arduino.h>
#include <FreeRTOS.h>
#include "../Utility/DF_GPIO.h"
#include "../OLED/ScreenManager.h"
#include "../WiFi/DFota.h"

// GPIO stubs (DF_GPIO.cpp is excluded from dieselfire build)
void CGPIOin::begin(int, int, CGPIOin1::Modes, CGPIOin2::Modes, int) {}
void CGPIOin::manage() {}
uint8_t CGPIOin::getState(int) { return 0; }
CGPIOin1::Modes CGPIOin::getMode1() const { return CGPIOin1::Disabled; }
CGPIOin2::Modes CGPIOin::getMode2() const { return CGPIOin2::Disabled; }
void CGPIOin::simulateKey(uint8_t) {}
CGPIOin::CGPIOin() {}

void CGPIOout::begin(int, int, CGPIOout1::Modes, CGPIOout2::Modes) {}
void CGPIOout::manage() {}
void CGPIOout::setThresh(int, int) {}
CGPIOout1::Modes CGPIOout::getMode1() const { return CGPIOout1::Disabled; }
CGPIOout2::Modes CGPIOout::getMode2() const { return CGPIOout2::Disabled; }
uint8_t CGPIOout::getState(int) { return 0; }
void CGPIOout::setState(int, bool) {}
CGPIOout::CGPIOout() {}

void CGPIOalg::begin(adc1_channel_t, CGPIOalg::Modes) {}
void CGPIOalg::manage() {}
int CGPIOalg::getValue() { return 0; }
CGPIOalg::Modes CGPIOalg::getMode() const { return CGPIOalg::Disabled; }
CGPIOalg::CGPIOalg() {}

// Screen manager stubs (OLED/ is excluded, replaced by LVGL)
CScreenManager::CScreenManager() {}
CScreenManager::~CScreenManager() {}
void CScreenManager::begin() {}
void CScreenManager::showBootMsg(const char*) {}
void CScreenManager::showRebootMsg(const char**, long) {}
void CScreenManager::showOTAMessage(int, eOTAmodes) {}
void CScreenManager::keyHandler(uint8_t) {}
void CScreenManager::reqUpdate() {}
bool CScreenManager::checkUpdate() { return false; }
bool CScreenManager::animate() { return false; }
void CScreenManager::refresh() {}
void CScreenManager::clearDisplay() {}
void CScreenManager::selectMenu(eUIMenuSets, int) {}

// SPIFFS listing (defined in DFWebServer.cpp)
// GPIO sub-component constructors
CGPIOin1::CGPIOin1() {}
CGPIOin2::CGPIOin2() {}
CGPIOout1::CGPIOout1() {}
CGPIOout2::CGPIOout2() {}
CGPIOin2::Modes CGPIOin2::getMode() const { return CGPIOin2::Disabled; }
const char* CGPIOin2::getExtThermTime() { return ""; }

// GPIO name tables (used by JSON formatter)
const char* GPIOin1Names[] = {"Disabled","Start","Run","StartStop","Stop","","",""};
const char* GPIOin2Names[] = {"Disabled","Stop","Thermostat","FuelReset","","","",""};
const char* GPIOout1Names[] = {"Disabled","User","OnWithHeater","OnWithGlow","","","",""};
const char* GPIOout2Names[] = {"Disabled","User","OnWithHeater","OnWithGlow","","","",""};
const char* GPIOalgNames[] = {"Disabled","ADC input","Temp NTC","Temp LM35","Temp DS18B20","","",""};

// GPIO base class constructor
CGPIOoutBase::CGPIOoutBase() {}

// Global ScreenManager instance (main.cpp skips it when USE_ILI9341_DISPLAY)
CScreenManager ScreenManager;

// Web server stubs (used when USE_WEBSERVER==1 but called from common code paths)
void setUploadSize(long) {}
TaskHandle_t handleWebServerTask = NULL;
