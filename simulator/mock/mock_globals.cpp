#include "cfg/DFConfig.h"
#include "cfg/pins.h"
#include "RTC/Clock.h"
#include "Utility/DebugPort.h"
#include "Utility/NVStorage.h"
#include "Utility/FuelGauge.h"
#include "Utility/DataFilter.h"
#include "Protocol/Protocol.h"

ClockClass Clock;
DebugPortClass DebugPort;
static CHeaterStorage _nvstore_impl;
CHeaterStorage& NVstore = _nvstore_impl;
CFuelGauge FuelGauge;
sFilteredData FilteredSamples;
CProtocolPackage BlueWireData;

const CProtocolPackage& getHeaterInfo() {
  static CProtocolPackage pkg;
  return pkg;
}
