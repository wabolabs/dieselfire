#include "MQTTScreen.h"
#include "Utility/NVStorage.h"

extern CHeaterStorage& NVstore;

MQTTScreen::MQTTScreen() : DieselScreen("MQTT") {}

void MQTTScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "MQTT", LV_ALIGN_TOP_LEFT, 48, 3);

  int y = 40;
  createLabel(_screen, "Status", LV_ALIGN_TOP_LEFT, 8, y);
  createValueLabel(_screen, "Disconnected", LV_ALIGN_TOP_LEFT, 60, y);

  y += 30;
  createLabel(_screen, "QoS", LV_ALIGN_TOP_LEFT, 8, y);
  createValueLabel(_screen, "0", LV_ALIGN_TOP_LEFT, 60, y);

  y += 30;
  createLabel(_screen, "Prefix", LV_ALIGN_TOP_LEFT, 8, y);
  createValueLabel(_screen, "DieselFire", LV_ALIGN_TOP_LEFT, 60, y);
}
