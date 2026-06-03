#include <cstdio>
#include "vserial.h"
#include "heater_emu.h"
#include "../src/Protocol/Protocol.h"
#include "../src/Protocol/BlueWireTask.h"

VirtualSerialPair g_serialPair;
VirtualSerial& BlueWireSerial = g_serialPair.a;
HeaterEmulator g_heater;

// Minimal tick that does nothing
void tickBlueWire(unsigned long) {}

extern void initBlueWire();

int main() {
  initBlueWire();
  g_heater.begin(&g_serialPair.b);
  g_heater.setSlowResponse(2);
  
  printf("Before: a.avail=%d b.avail=%d\n", 
         g_serialPair.a.available(), g_serialPair.b.available());
  fflush(stdout);
  
  tickBlueWire(0);
  
  printf("After: a.avail=%d b.avail=%d\n", 
         g_serialPair.a.available(), g_serialPair.b.available());
  fflush(stdout);
  
  g_heater.tick(5);
  
  printf("After tick: a.avail=%d b.avail=%d\n", 
         g_serialPair.a.available(), g_serialPair.b.available());
  printf("Done\n");
  return 0;
}
