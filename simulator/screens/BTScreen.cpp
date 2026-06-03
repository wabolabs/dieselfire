#include "BTScreen.h"

BTScreen::BTScreen() : DieselScreen("Bluetooth") {}

void BTScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Bluetooth", LV_ALIGN_TOP_LEFT, 48, 3);

  int y = 40;
  createLabel(_screen, "Status", LV_ALIGN_TOP_LEFT, 8, y);
  createValueLabel(_screen, "BLE Active", LV_ALIGN_TOP_LEFT, 60, y);

  y += 30;
  createLabel(_screen, "MAC", LV_ALIGN_TOP_LEFT, 8, y);
  createValueLabel(_screen, "AA:BB:CC:DD:EE:FF", LV_ALIGN_TOP_LEFT, 60, y);

  y += 30;
  createLabel(_screen, "Type", LV_ALIGN_TOP_LEFT, 8, y);
  createValueLabel(_screen, "ESP32 BLE", LV_ALIGN_TOP_LEFT, 60, y);
}
