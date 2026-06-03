#include <cstdio>
#include "vserial.h"
#include "heater_emu.h"

extern void initBlueWire();
extern void tickBlueWire(unsigned long);

VirtualSerialPair g_serialPair;
VirtualSerial& BlueWireSerial = g_serialPair.a;
HeaterEmulator g_heater;

int main() {
  initBlueWire();
  g_heater.begin(&g_serialPair.b);
  g_heater.setSlowResponse(2);
  
  printf("Before tickBlueWire: a.avail=%d b.avail=%d\n", 
         g_serialPair.a.available(), g_serialPair.b.available());
  fflush(stdout);
  
  tickBlueWire(0);
  
  printf("After tickBlueWire(0): a.avail=%d b.avail=%d\n", 
         g_serialPair.a.available(), g_serialPair.b.available());
  fflush(stdout);
  
  g_heater.tick(5);
  
  printf("After heater.tick: a.avail=%d b.avail=%d\n", 
         g_serialPair.a.available(), g_serialPair.b.available());
  printf("Done\n");
  return 0;
}
