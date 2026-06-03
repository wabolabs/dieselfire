#include "HeaterSettingsScreen.h"
#include "../../Utility/NVStorage.h"
#include <cstdio>

extern CHeaterStorage& NVstore;

static const char VOLT_OPTS[] = "12V\n24V";
static const char FAN_OPTS[] = "SN-1\nSN-2";

HeaterSettingsScreen::HeaterSettingsScreen() : DieselScreen("HeaterSet") {}

void HeaterSettingsScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Heater Settings", LV_ALIGN_TOP_LEFT, 48, 3);

  auto& t = NVstore.getHeaterTuning();
  int y = 36;

  createLabel(_screen, "Voltage", LV_ALIGN_TOP_LEFT, 8, y);
  _voltDropdown = lv_dropdown_create(_screen);
  lv_obj_set_pos(_voltDropdown, 80, y); lv_obj_set_width(_voltDropdown, TFT_WIDTH - 90);
  lv_dropdown_set_options_static(_voltDropdown, VOLT_OPTS);
  lv_dropdown_set_selected(_voltDropdown, t.sysVoltage == 240 ? 1 : 0);
  lv_obj_add_event_cb(_voltDropdown, onChange, LV_EVENT_VALUE_CHANGED, this);
  y += 40;

  createLabel(_screen, "Fan Sensor", LV_ALIGN_TOP_LEFT, 8, y);
  _fanDropdown = lv_dropdown_create(_screen);
  lv_obj_set_pos(_fanDropdown, 80, y); lv_obj_set_width(_fanDropdown, TFT_WIDTH - 90);
  lv_dropdown_set_options_static(_fanDropdown, FAN_OPTS);
  lv_dropdown_set_selected(_fanDropdown, t.fanSensor - 1);
  lv_obj_add_event_cb(_fanDropdown, onChange, LV_EVENT_VALUE_CHANGED, this);
  y += 40;

  createLabel(_screen, "Glow Drive", LV_ALIGN_TOP_LEFT, 8, y);
  _glowSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_glowSlider, 90, y); lv_obj_set_size(_glowSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_glowSlider, 1, 6);
  lv_slider_set_value(_glowSlider, t.glowDrive, LV_ANIM_OFF);
  lv_obj_add_event_cb(_glowSlider, onChange, LV_EVENT_VALUE_CHANGED, this);
  _glowLabel = lv_label_create(_screen);
  lv_obj_align_to(_glowLabel, _glowSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  char buf[16]; snprintf(buf, sizeof(buf), "PF-%d", t.glowDrive); lv_label_set_text(_glowLabel, buf);
}

void HeaterSettingsScreen::onChange(lv_event_t* e) {
  static_cast<HeaterSettingsScreen*>(lv_event_get_user_data(e))->save();
}

void HeaterSettingsScreen::save() {
  auto t = NVstore.getHeaterTuning();
  t.sysVoltage = lv_dropdown_get_selected(_voltDropdown) == 1 ? 240 : 120;
  t.fanSensor = lv_dropdown_get_selected(_fanDropdown) + 1;
  t.glowDrive = lv_slider_get_value(_glowSlider);
  NVstore.setHeaterTuning(t);
  NVstore.save();

  char buf[16];
  snprintf(buf, sizeof(buf), "PF-%d", t.glowDrive); lv_label_set_text(_glowLabel, buf);
}
