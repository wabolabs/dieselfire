#include <Arduino.h>
#include "cfg/DFConfig.h"
#include "cfg/pins.h"
#include "RTC/Clock.h"
#include "Utility/DebugPort.h"
#include "Utility/NVStorage.h"
#include "Utility/FuelGauge.h"
#include "Utility/DataFilter.h"
#include "Utility/TempSense.h"
#include "Utility/DF_GPIO.h"
#include "Protocol.h"
#include "Protocol/BlueWireTask.h"

ClockClass Clock;
DebugPortClass DebugPort;
static CHeaterStorage _nvstore_impl;
CHeaterStorage& NVstore = _nvstore_impl;
CFuelGauge FuelGauge;
sFilteredData FilteredSamples;
CProtocolPackage BlueWireData;
CTempSense TempSensor;
CGPIOin GPIOin;
CGPIOout GPIOout;
CGPIOalg GPIOalg;

const char* GPIOin1Names[] = {"Disabled","Start","Run","StartStop","Stop","","",""};
const char* GPIOin2Names[] = {"Disabled","Stop","Thermostat","FuelReset","","","",""};
const char* GPIOout1Names[] = {"Disabled","User","OnWithHeater","OnWithGlow","","","",""};
const char* GPIOout2Names[] = {"Disabled","User","OnWithHeater","OnWithGlow","","","",""};
const char* GPIOalgNames[] = {"Disabled","ADC","NTC","LM35","DS18B20","","",""};

// Report flags (toggled from debug console in real firmware)
bool bReportRecyleEvents = 1;
bool bReportOEMresync = 0;
bool bReportBlueWireData = 0;

// Stubs for functions declared in helpers.h, defined in main.cpp on real hardware
int getSmartError() { return 0; }
bool isCyclicStopStartActive() { return false; }

// BlueWire task queue handles — populated by bluwire_task.cpp
extern QueueHandle_t BlueWireMsgQueue;
extern QueueHandle_t BlueWireRxQueue;
extern QueueHandle_t BlueWireTxQueue;
extern SemaphoreHandle_t BlueWireSemaphore;

// Heater data from the real CProtocolPackage (populated via checkBlueWireEvents)
const CProtocolPackage& getHeaterInfo() {
  return BlueWireData;
}

// Stub for checkBlueWireEvents — called from main loop.
void checkBlueWireEvents() {
  static char taskMsg[BLUEWIRE_MSGQUEUESIZE];

  if (BlueWireMsgQueue && xQueueReceive(BlueWireMsgQueue, taskMsg, 0))
    DebugPort.print(taskMsg);

  if (BlueWireSemaphore && xSemaphoreTake(BlueWireSemaphore, 0)) {
    // Exchange completed — process queued data below
  }

  uint8_t txBuf[24];
  if (BlueWireTxQueue && xQueueReceive(BlueWireTxQueue, txBuf, 0)) {
    // TX frame sent (monitoring not needed for UI)
  }

  uint8_t rxBuf[24];
  if (BlueWireRxQueue && xQueueReceive(BlueWireRxQueue, rxBuf, 0)) {
    CProtocol rxProto(CProtocol::HeatMode);
    memcpy(rxProto.Data, rxBuf, 24);
    CProtocol txProto(CProtocol::CtrlMode);
    memcpy(txProto.Data, txBuf, 24);
    BlueWireData.set(rxProto, txProto);
  }
}
