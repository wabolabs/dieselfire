#include <cstdio>
#include "vserial.h"
#include "heater_emu.h"

// Include the global definitions directly
CommStates CommState;
CProtocol DefaultDFParams(CProtocol::CtrlMode);
CModeratedFrame OEMCtrlFrame;
CModeratedFrame HeaterFrame1;
CProtocol HeaterFrame2;
// ... etc

extern void initBlueWire();
extern void tickBlueWire(unsigned long);

VirtualSerialPair g_serialPair;
VirtualSerial& BlueWireSerial = g_serialPair.a;
HeaterEmulator g_heater;

int main() {
  printf("sizeof CProtocol = %zu\n", sizeof(CProtocol));
  printf("sizeof CommStates = %zu\n", sizeof(CommStates));
  printf("Addresses:\n");
  printf("  g_serialPair: %p\n", &g_serialPair);
  printf("  g_heater: %p\n", &g_heater);
  printf("  DefaultDFParams: %p\n", &DefaultDFParams);
  printf("  BlueWireRxDataLocal: %p\n", &BlueWireRxDataLocal);
  printf("  BlueWireMsgQueue: %p\n", &BlueWireMsgQueue);
  
  initBlueWire();
  g_heater.begin(&g_serialPair.b);
  
  printf("\nBefore: a._txBuf at %p, b._txBuf at %p\n", 
         (void*)&g_serialPair.a._txBuf, (void*)&g_serialPair.b._txBuf);
  printf("Before: a._rxBuf=%p, b._rxBuf=%p\n",
         (void*)g_serialPair.a._rxBuf, (void*)g_serialPair.b._rxBuf);
  fflush(stdout);
  
  tickBlueWire(0);
  
  printf("\nAfter tickBlueWire(0):\n");
  printf("  a.avail=%d b.avail=%d\n", 
         g_serialPair.a.available(), g_serialPair.b.available());
  
  return 0;
}
