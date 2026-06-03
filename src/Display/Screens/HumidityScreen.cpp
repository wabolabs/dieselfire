#include "HumidityScreen.h"
#include "../../Utility/NVStorage.h"

extern CHeaterStorage& NVstore;

HumidityScreen::HumidityScreen() : DieselScreen("Humidity") {}

void HumidityScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Humidity Trigger", LV_ALIGN_TOP_LEFT, 48, 3);

  auto& s = NVstore.getUserSettings();
  int y = 40;

  createLabel(_screen, "Start >", LV_ALIGN_TOP_LEFT, 8, y);
  createLabel(_screen, "%RH", LV_ALIGN_TOP_LEFT, 8, y + 18);

  _slider = lv_slider_create(_screen);
  lv_obj_set_pos(_slider, 60, y);
  lv_obj_set_size(_slider, TFT_WIDTH - 110, 16);
  lv_slider_set_range(_slider, 0, 100);
  lv_slider_set_value(_slider, s.humidityStart, LV_ANIM_OFF);
  lv_obj_add_event_cb(_slider, onSliderChange, LV_EVENT_VALUE_CHANGED, this);

  _label = lv_label_create(_screen);
  lv_obj_align_to(_label, _slider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  char buf[16];
  if (s.humidityStart == 0) lv_label_set_text(_label, "OFF");
  else { snprintf(buf, sizeof(buf), "%d%%", s.humidityStart); lv_label_set_text(_label, buf); }
}

void HumidityScreen::onSliderChange(lv_event_t* e) {
  static_cast<HumidityScreen*>(lv_event_get_user_data(e))->save();
}

void HumidityScreen::save() {
  auto s = NVstore.getUserSettings();
  s.humidityStart = lv_slider_get_value(_slider);
  NVstore.setUserSettings(s);
  NVstore.save();
  char buf[16];
  if (s.humidityStart == 0) lv_label_set_text(_label, "OFF");
  else { snprintf(buf, sizeof(buf), "%d%%", s.humidityStart); lv_label_set_text(_label, buf); }
}
