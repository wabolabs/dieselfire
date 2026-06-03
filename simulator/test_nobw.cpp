#include <cstdio>
#include "vserial.h"
#include "heater_emu.h"

VirtualSerialPair g_serialPair;
VirtualSerial& BlueWireSerial = g_serialPair.a;
HeaterEmulator g_heater;

int main() {
  // NO initBlueWire()
  g_heater.begin(&g_serialPair.b);
  g_heater.setSlowResponse(2);
  
  printf("a.avail=%d b.avail=%d\n", g_serialPair.a.available(), g_serialPair.b.available());
  fflush(stdout);
  
  // Write something from BlueWire side
  uint8_t data[] = {0x76, 0x16, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  BlueWireSerial.write(data, 24);
  
  printf("After write: a.avail=%d b.avail=%d\n", g_serialPair.a.available(), g_serialPair.b.available());
  
  g_heater.tick(5);
  
  printf("After tick: a.avail=%d b.avail=%d\n", g_serialPair.a.available(), g_serialPair.b.available());
  printf("Done\n");
  return 0;
}
