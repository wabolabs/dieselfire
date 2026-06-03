#include "SystemScreen.h"
#include "Utility/NVStorage.h"
#include "Protocol/Protocol.h"
#include <cstdio>

extern CHeaterStorage& NVstore;

SystemScreen::SystemScreen() : DieselScreen("System") {}

void SystemScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "System Info", LV_ALIGN_TOP_LEFT, 48, 3);

  int y = 36;
  createLabel(_screen, "Version", LV_ALIGN_TOP_LEFT, 8, y);
  _verLabel = createValueLabel(_screen, "DieselFire 1.0", LV_ALIGN_TOP_LEFT, 70, y);

  y += 30;
  createLabel(_screen, "Board", LV_ALIGN_TOP_LEFT, 8, y);
  _boardLabel = createValueLabel(_screen, "Rev 3.0", LV_ALIGN_TOP_LEFT, 70, y);

  y += 30;
  createLabel(_screen, "WiFi Mode", LV_ALIGN_TOP_LEFT, 8, y);
  auto& s = NVstore.getUserSettings();
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", s.wifiMode);
  createValueLabel(_screen, buf, LV_ALIGN_TOP_LEFT, 70, y);

  y += 30;
  createLabel(_screen, "Menu Mode", LV_ALIGN_TOP_LEFT, 8, y);
  snprintf(buf, sizeof(buf), "%d", s.menuMode);
  createValueLabel(_screen, buf, LV_ALIGN_TOP_LEFT, 70, y);
}

void SystemScreen::onTimer() {}
