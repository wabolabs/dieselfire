#include <cstdio>
#include "vserial.h"
#include "heater_emu.h"

extern void initBlueWire();
extern void tickBlueWire(unsigned long);
extern void checkBlueWireEvents();

VirtualSerialPair g_serialPair;
VirtualSerial& BlueWireSerial = g_serialPair.a;
HeaterEmulator g_heater;

int main() {
  printf("Starting\n");
  initBlueWire();
  g_heater.begin(&g_serialPair.b);
  g_heater.setSlowResponse(2);
  
  printf("Running ticks...\n");
  fflush(stdout);
  
  for (int i = 0; i < 30; i++) {
    unsigned long now = (unsigned long)i * 50;
    tickBlueWire(now);
    g_heater.tick(5);
    checkBlueWireEvents();
    
    int a_avail = g_serialPair.a.available();
    int b_avail = g_serialPair.b.available();
    printf("  tick %d: a_avail=%d b_avail=%d state=%d\n", 
           i, a_avail, b_avail, g_heater.getRunState());
    fflush(stdout);
    
    if (a_avail > 100 || b_avail > 100) {
      printf("CORRUPTION\n");
      break;
    }
  }
  
  printf("Done\n");
  return 0;
}
