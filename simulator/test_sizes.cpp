#include <cstdio>
#include "../src/Protocol/Protocol.h"
#include "../src/Protocol/BlueWireTask.h"
#include "../src/Utility/UtilClasses.h"
#include "heater_emu.h"
#include "vserial.h"

int main() {
  printf("sizeof(CProtocol) = %zu\n", sizeof(CProtocol));
  printf("sizeof(CModeratedFrame) = %zu\n", sizeof(CModeratedFrame));
  printf("sizeof(CommStates) = %zu\n", sizeof(CommStates));
  printf("sizeof(CProtocolPackage) = %zu\n", sizeof(CProtocolPackage));
  printf("sizeof(sRxData) = %zu\n", sizeof(sRxData));
  printf("sizeof(VirtualSerial) = %zu\n", sizeof(VirtualSerial));
  printf("sizeof(VirtualSerialPair) = %zu\n", sizeof(VirtualSerialPair));
  printf("sizeof(HeaterEmulator) = %zu\n", sizeof(HeaterEmulator));
  printf("sizeof(std::vector<uint8_t>) = %zu\n", sizeof(std::vector<uint8_t>));
  return 0;
}
