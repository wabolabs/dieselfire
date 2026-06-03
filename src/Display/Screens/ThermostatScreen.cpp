#include "ThermostatScreen.h"
#include "../../Utility/NVStorage.h"

extern CHeaterStorage& NVstore;

static const char METHODS_STR[] = "Standard\nDeadband\nLinear Hz\nExt Contact\nStop/Start";

ThermostatScreen::ThermostatScreen() : DieselScreen("Thermostat") {}

void ThermostatScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Thermostat", LV_ALIGN_TOP_LEFT, 48, 3);

  auto& s = NVstore.getUserSettings();
  int y = 30;

  createLabel(_screen, "Method", LV_ALIGN_TOP_LEFT, 8, y);
  _methodDropdown = lv_dropdown_create(_screen);
  lv_obj_set_pos(_methodDropdown, 80, y);
  lv_obj_set_width(_methodDropdown, TFT_WIDTH - 90);
  lv_dropdown_set_options_static(_methodDropdown, METHODS_STR);
  lv_dropdown_set_selected(_methodDropdown, s.ThermostatMethod);
  lv_obj_add_event_cb(_methodDropdown, onValueChange, LV_EVENT_VALUE_CHANGED, this);

  y += 36;
  createLabel(_screen, "Window", LV_ALIGN_TOP_LEFT, 8, y);
  _windowSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_windowSlider, 80, y);
  lv_obj_set_size(_windowSlider, TFT_WIDTH - 130, 16);
  lv_slider_set_range(_windowSlider, 2, 100);
  lv_slider_set_value(_windowSlider, s.ThermostatWindow * 10, LV_ANIM_OFF);
  lv_obj_add_event_cb(_windowSlider, onValueChange, LV_EVENT_VALUE_CHANGED, this);
  _windowLabel = lv_label_create(_screen);
  lv_obj_align_to(_windowLabel, _windowSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", s.ThermostatWindow);
  lv_label_set_text(_windowLabel, buf);

  y += 36;
  createLabel(_screen, "Cyc Stop>", LV_ALIGN_TOP_LEFT, 8, y);
  _stopSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_stopSlider, 80, y);
  lv_obj_set_size(_stopSlider, TFT_WIDTH - 130, 16);
  lv_slider_set_range(_stopSlider, 0, 100);
  lv_slider_set_value(_stopSlider, s.cyclic.Stop * 10, LV_ANIM_OFF);
  lv_obj_add_event_cb(_stopSlider, onValueChange, LV_EVENT_VALUE_CHANGED, this);
  _stopLabel = lv_label_create(_screen);
  lv_obj_align_to(_stopLabel, _stopSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  snprintf(buf, sizeof(buf), "%d", s.cyclic.Stop);
  lv_label_set_text(_stopLabel, buf);

  y += 36;
  createLabel(_screen, "Cyc Start<", LV_ALIGN_TOP_LEFT, 8, y);
  _startSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_startSlider, 80, y);
  lv_obj_set_size(_startSlider, TFT_WIDTH - 130, 16);
  lv_slider_set_range(_startSlider, -100, 0);
  lv_slider_set_value(_startSlider, s.cyclic.Start * 10, LV_ANIM_OFF);
  lv_obj_add_event_cb(_startSlider, onValueChange, LV_EVENT_VALUE_CHANGED, this);
  _startLabel = lv_label_create(_screen);
  lv_obj_align_to(_startLabel, _startSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  snprintf(buf, sizeof(buf), "%d", s.cyclic.Start);
  lv_label_set_text(_startLabel, buf);
}

void ThermostatScreen::onValueChange(lv_event_t* e) {
  auto* self = static_cast<ThermostatScreen*>(lv_event_get_user_data(e));
  self->saveSettings();
}

void ThermostatScreen::saveSettings() {
  auto s = NVstore.getUserSettings();
  s.ThermostatMethod = lv_dropdown_get_selected(_methodDropdown);
  s.ThermostatWindow = (float)lv_slider_get_value(_windowSlider) / 10.0f;
  s.cyclic.Stop = (int8_t)(lv_slider_get_value(_stopSlider) / 10);
  s.cyclic.Start = (int8_t)(lv_slider_get_value(_startSlider) / 10);
  NVstore.setUserSettings(s);
  NVstore.save();

  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", s.ThermostatWindow);
  lv_label_set_text(_windowLabel, buf);
  snprintf(buf, sizeof(buf), "%d", s.cyclic.Stop);
  lv_label_set_text(_stopLabel, buf);
  snprintf(buf, sizeof(buf), "%d", s.cyclic.Start);
  lv_label_set_text(_startLabel, buf);
}
