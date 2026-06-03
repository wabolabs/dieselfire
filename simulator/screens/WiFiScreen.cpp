#include "WiFiScreen.h"
#include "Utility/NVStorage.h"

extern CHeaterStorage& NVstore;

WiFiScreen::WiFiScreen() : DieselScreen("WiFi") {}

void WiFiScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "WiFi Status", LV_ALIGN_TOP_LEFT, 48, 3);

  int y = 40;
  createLabel(_screen, "Mode", LV_ALIGN_TOP_LEFT, 8, y);
  _modeLabel = createValueLabel(_screen, "", LV_ALIGN_TOP_LEFT, 60, y);

  y += 30;
  createLabel(_screen, "IP", LV_ALIGN_TOP_LEFT, 8, y);
  _ipLabel = createValueLabel(_screen, "", LV_ALIGN_TOP_LEFT, 60, y);

  y += 30;
  createLabel(_screen, "RSSI", LV_ALIGN_TOP_LEFT, 8, y);
  _rssiLabel = createValueLabel(_screen, "", LV_ALIGN_TOP_LEFT, 60, y);

  y += 30;
  createLabel(_screen, "OTA", LV_ALIGN_TOP_LEFT, 8, y);
  auto& s = NVstore.getUserSettings();
  createValueLabel(_screen, s.enableOTA ? "Enabled" : "Disabled", LV_ALIGN_TOP_LEFT, 60, y);

  _timer = lv_timer_create([](lv_timer_t* t) {
    auto* s = static_cast<WiFiScreen*>(lv_timer_get_user_data(t));
    if (s) s->onTimer();
  }, 2000, this);
}

void WiFiScreen::onTimer() { updateData(); }

void WiFiScreen::updateData() {
  auto& s = NVstore.getUserSettings();
  static const char* modes[] = {"Off","CFG AP","AP","CFG STA+AP","STA+AP","CFG STA","STA"};
  const char* mode = (s.wifiMode < 7) ? modes[s.wifiMode] : "?";
  lv_label_set_text(_modeLabel, mode);
  lv_label_set_text(_ipLabel, s.wifiMode > 2 ? "192.168.x.x" : "N/A");
  lv_label_set_text(_rssiLabel, s.wifiMode > 2 ? "-50 dBm" : "N/A");
}
