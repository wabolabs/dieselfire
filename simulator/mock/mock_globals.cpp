#include "cfg/DFConfig.h"
#include "cfg/pins.h"
#include "RTC/Clock.h"
#include "Utility/DebugPort.h"
#include "Utility/NVStorage.h"
#include "Utility/FuelGauge.h"
#include "Utility/DataFilter.h"
#include "Utility/TempSense.h"
#include "Utility/DF_GPIO.h"
#include "Protocol/Protocol.h"

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

const CProtocolPackage& getHeaterInfo() {
  static CProtocolPackage pkg;
  return pkg;
}
