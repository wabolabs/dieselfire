#include <cstdio>
#include "vserial.h"
#include "heater_emu.h"

VirtualSerialPair g_serialPair;
VirtualSerial& BlueWireSerial = g_serialPair.a;
HeaterEmulator g_heater;

int main() {
  printf("Starting\n");
  g_heater.begin(&g_serialPair.b);
  
  uint8_t data[] = {0x76, 0x16, 0x00, 0x00};
  BlueWireSerial.write(data, 4);
  
  printf("After write, BW avail=%d, Htr avail=%d\n", 
         BlueWireSerial.available(), g_serialPair.b.available());
  
  while (g_serialPair.b.available()) {
    int c = g_serialPair.b.read();
    printf("Read: %02X\n", c);
  }
  
  printf("Done\n");
  return 0;
}
