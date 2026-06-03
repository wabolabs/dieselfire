#include <cstdio>
#include "vserial.h"
#include "heater_emu.h"

extern void initBlueWire();

VirtualSerialPair g_serialPair;
VirtualSerial& BlueWireSerial = g_serialPair.a;
HeaterEmulator g_heater;

int main() {
  printf("Step 1: after globals constructed\n");
  printf("  a.avail=%d b.avail=%d\n", g_serialPair.a.available(), g_serialPair.b.available());
  
  initBlueWire();
  printf("Step 2: after initBlueWire\n");
  printf("  a.avail=%d b.avail=%d\n", g_serialPair.a.available(), g_serialPair.b.available());
  
  g_heater.begin(&g_serialPair.b);
  printf("Step 3: after heater.begin\n");
  printf("  a.avail=%d b.avail=%d\n", g_serialPair.a.available(), g_serialPair.b.available());
  
  g_heater.setSlowResponse(2);
  
  printf("Step 4: checking avail\n");
  int a = g_serialPair.a.available();
  int b = g_serialPair.b.available();
  printf("  a.avail=%d b.avail=%d\n", a, b);
  fflush(stdout);
  
  g_heater.tick(5);
  printf("Step 5: after heater tick\n");
  printf("  a.avail=%d b.avail=%d\n", g_serialPair.a.available(), g_serialPair.b.available());
  
  printf("Done\n");
  return 0;
}
