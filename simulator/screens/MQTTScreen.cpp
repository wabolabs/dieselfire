#include "MQTTScreen.h"
#include "WiFi/DFMQTT.h"
#include "Utility/NVStorage.h"
#include <cstdio>

extern CHeaterStorage& NVstore;

MQTTScreen::MQTTScreen() : DieselScreen("MQTT") {}

void MQTTScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "MQTT", LV_ALIGN_TOP_LEFT, 48, 3);

  int y = 40;
  createLabel(_screen, "Status", LV_ALIGN_TOP_LEFT, 8, y);
  _valStatus = createValueLabel(_screen, "?", LV_ALIGN_TOP_LEFT, 60, y);

  y += 28;
  createLabel(_screen, "Broker", LV_ALIGN_TOP_LEFT, 8, y);
  _valHost = createValueLabel(_screen, "?", LV_ALIGN_TOP_LEFT, 60, y);

  y += 28;
  createLabel(_screen, "Prefix", LV_ALIGN_TOP_LEFT, 8, y);
  _valPrefix = createValueLabel(_screen, "?", LV_ALIGN_TOP_LEFT, 60, y);

  y += 28;
  createLabel(_screen, "HA Disc.", LV_ALIGN_TOP_LEFT, 8, y);
  _valHA = createValueLabel(_screen, "?", LV_ALIGN_TOP_LEFT, 60, y);

  _timer = lv_timer_create([](lv_timer_t* t) {
    static_cast<MQTTScreen*>(lv_timer_get_user_data(t))->onTimer();
  }, 2000, this);
}

void MQTTScreen::onTimer() {
  const sMQTTparams& m = NVstore.getMQTTinfo();
  char buf[64];

  lv_label_set_text(_valStatus, m.enabled ? (isMQTTconnected() ? "Connected" : "Disconnected") : "Disabled");

  snprintf(buf, sizeof(buf), "%s:%d", m.host[0] ? m.host : "(none)", m.port);
  lv_label_set_text(_valHost, buf);

  lv_label_set_text(_valPrefix, getTopicPrefix());

  snprintf(buf, sizeof(buf), "%s", m.haDiscovery ? "Enabled" : "Disabled");
  lv_label_set_text(_valHA, buf);
}
