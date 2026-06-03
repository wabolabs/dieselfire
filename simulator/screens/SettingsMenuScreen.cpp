#include "SettingsMenuScreen.h"
#include "ThermostatScreen.h"
#include "FrostScreen.h"
#include "HumidityScreen.h"
#include "TimeoutsScreen.h"
#include "TimerChartScreen.h"
#include "TimerEditScreen.h"
#include "WiFiScreen.h"
#include "MQTTScreen.h"
#include "BTScreen.h"
#include "cfg/DFConfig.h"

SettingsMenuScreen::SettingsMenuScreen() : DieselScreen("Settings") {}

void SettingsMenuScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Settings", LV_ALIGN_TOP_LEFT, 48, 4);
  hideBackButton();

  static const char* items[] = {
    "Timer Overview", "Edit Timer",
    "WiFi", "MQTT", "Bluetooth",
    "Thermostat", "Frost", "Humidity", "Timeouts",
    nullptr
  };

  lv_obj_t* list = lv_list_create(_screen);
  lv_obj_set_size(list, TFT_WIDTH, TFT_HEIGHT - 22);
  lv_obj_set_pos(list, 0, 22);
  lv_obj_set_style_bg_color(list, lv_color_make(0x12, 0x12, 0x12), 0);
  lv_obj_set_style_border_width(list, 0, 0);

  for (int i = 0; items[i]; i++) {
    lv_obj_t* btn = lv_list_add_button(list, nullptr, items[i]);
    lv_obj_set_user_data(btn, (void*)(intptr_t)i);
    lv_obj_add_event_cb(btn, onEntryClick, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btn, lv_color_make(0x22, 0x22, 0x22), 0);
    lv_obj_set_style_bg_color(btn, lv_color_make(0xFF, 0x7A, 0x00), LV_STATE_PRESSED);
  }
}

void SettingsMenuScreen::onEntryClick(lv_event_t* e) {
  auto* self = static_cast<SettingsMenuScreen*>(lv_event_get_user_data(e));
  auto* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
  int idx = (int)(intptr_t)lv_obj_get_user_data(btn);
  self->openScreen(idx);
}

void SettingsMenuScreen::openScreen(int index) {
  DieselScreen* next = nullptr;
  switch (index) {
    case 0: next = new TimerChartScreen(); break;
    case 1: next = new TimerEditScreen(); break;
    case 2: next = new WiFiScreen(); break;
    case 3: next = new MQTTScreen(); break;
    case 4: next = new BTScreen(); break;
    case 5: next = new ThermostatScreen(); break;
    case 6: next = new FrostScreen(); break;
    case 7: next = new HumidityScreen(); break;
    case 8: next = new TimeoutsScreen(); break;
  }
  if (next) {
    next->onLoad();
    lv_scr_load(next->getScreen());
  }
}
