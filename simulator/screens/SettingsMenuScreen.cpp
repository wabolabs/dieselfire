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
#include "SensorScreen.h"
#include "GPIOScreen.h"
#include "FuelMixtureScreen.h"
#include "HeaterSettingsScreen.h"
#include "FuelCalScreen.h"
#include "LVCScreen.h"
#include "SystemScreen.h"
#include "RebootScreen.h"
#include "HourMeterScreen.h"
#include "InheritScreen.h"
#include "MainStatusScreen.h"
#include "cfg/DFConfig.h"

SettingsMenuScreen::SettingsMenuScreen() : DieselScreen("Settings") {}

void SettingsMenuScreen::deleteSubScreen() {
  if (_subScreen) {
    lv_obj_del(_subScreen->getScreen());
    delete _subScreen;
    _subScreen = nullptr;
  }
}

void SettingsMenuScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Settings", LV_ALIGN_TOP_LEFT, 48, 4);

  static const char* items[] = {
    "Timer Overview", "Edit Timer",
    "WiFi", "MQTT", "Bluetooth",
    "Sensors", "GPIO",
    "Fuel Mixture", "Heater Settings", "Fuel Cal", "LVC",
    "System Info", "Hour Meter", "Inherit", "Reboot",
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
    lv_obj_set_height(btn, 32);
    lv_obj_set_style_pad_top(btn, 6, 0);
    lv_obj_set_style_bg_color(btn, lv_color_make(0x22, 0x22, 0x22), 0);
    lv_obj_set_style_bg_color(btn, lv_color_make(0xFF, 0x7A, 0x00), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_t* lbl = lv_obj_get_child(btn, 0);
    if (lbl) lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
  }
}

void SettingsMenuScreen::onEntryClick(lv_event_t* e) {
  auto* self = static_cast<SettingsMenuScreen*>(lv_event_get_user_data(e));
  auto* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
  int idx = (int)(intptr_t)lv_obj_get_user_data(btn);
  self->openScreen(idx);
}

void SettingsMenuScreen::onBack() {
  // Return to main status screen
  deleteSubScreen();
  auto* main = new MainStatusScreen();
  main->onLoad();
  lv_scr_load(main->getScreen());
}

void SettingsMenuScreen::openScreen(int index) {
  // Clean up any previous sub-screen
  deleteSubScreen();

  DieselScreen* next = nullptr;
  switch (index) {
    case 0: next = new TimerChartScreen(); break;
    case 1: next = new TimerEditScreen(); break;
    case 2: next = new WiFiScreen(); break;
    case 3: next = new MQTTScreen(); break;
    case 4: next = new BTScreen(); break;
    case 5: next = new SensorScreen(); break;
    case 6: next = new GPIOScreen(); break;
    case 7: next = new FuelMixtureScreen(); break;
    case 8: next = new HeaterSettingsScreen(); break;
    case 9: next = new FuelCalScreen(); break;
    case 10: next = new LVCScreen(); break;
    case 11: next = new SystemScreen(); break;
    case 12: next = new HourMeterScreen(); break;
    case 13: next = new InheritScreen(); break;
    case 14: next = new RebootScreen(); break;
    case 15: next = new ThermostatScreen(); break;
    case 16: next = new FrostScreen(); break;
    case 17: next = new HumidityScreen(); break;
    case 18: next = new TimeoutsScreen(); break;
  }
  if (next) {
    _subScreen = next;
    // Back callback: load settings screen, then clean up sub-screen
    SettingsMenuScreen* sm = this;
    next->setBackCallback([sm, next]() {
      lv_obj_t* subScr = next->getScreen();
      lv_scr_load(sm->getScreen());
      lv_obj_del(subScr);
      sm->_subScreen = nullptr;
      delete next;
    });
    next->onLoad();
    lv_scr_load(next->getScreen());
  }
}
