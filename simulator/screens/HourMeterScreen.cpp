#include "HourMeterScreen.h"
#include "Utility/NVStorage.h"
#include <cstdio>

extern CHeaterStorage& NVstore;

HourMeterScreen::HourMeterScreen() : DieselScreen("HourMeter") {}

void HourMeterScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Hour Meter", LV_ALIGN_TOP_LEFT, 48, 3);

  int y = 40;
  createLabel(_screen, "Run", LV_ALIGN_TOP_LEFT, 8, y);
  _runLabel = createValueLabel(_screen, "0h 0m", LV_ALIGN_TOP_LEFT, 60, y);

  y += 30;
  createLabel(_screen, "Glow", LV_ALIGN_TOP_LEFT, 8, y);
  _glowLabel = createValueLabel(_screen, "0h 0m", LV_ALIGN_TOP_LEFT, 60, y);

  y += 30;
  createLabel(_screen, "Uptime", LV_ALIGN_TOP_LEFT, 8, y);
  _upLabel = createValueLabel(_screen, "0h 0m", LV_ALIGN_TOP_LEFT, 60, y);
}
