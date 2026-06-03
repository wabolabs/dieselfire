#include "FrostScreen.h"
#include "../../Utility/NVStorage.h"

extern CHeaterStorage& NVstore;

FrostScreen::FrostScreen() : DieselScreen("Frost") {}

void FrostScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Frost Protection", LV_ALIGN_TOP_LEFT, 48, 3);

  auto& s = NVstore.getUserSettings();
  int y = 40;

  createLabel(_screen, "Auto-on <", LV_ALIGN_TOP_LEFT, 8, y);
  _onSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_onSlider, 90, y);
  lv_obj_set_size(_onSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_onSlider, 0, 100);
  lv_slider_set_value(_onSlider, s.FrostOn, LV_ANIM_OFF);
  lv_obj_add_event_cb(_onSlider, onValueChange, LV_EVENT_VALUE_CHANGED, this);
  _onLabel = lv_label_create(_screen);
  lv_obj_align_to(_onLabel, _onSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  char buf[16];
  if (s.FrostOn == 0) lv_label_set_text(_onLabel, "OFF");
  else { snprintf(buf, sizeof(buf), "%d", s.FrostOn); lv_label_set_text(_onLabel, buf); }

  y += 40;
  createLabel(_screen, "Auto-off >", LV_ALIGN_TOP_LEFT, 8, y);
  _riseSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_riseSlider, 90, y);
  lv_obj_set_size(_riseSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_riseSlider, 1, 20);
  lv_slider_set_value(_riseSlider, s.FrostRise, LV_ANIM_OFF);
  lv_obj_add_event_cb(_riseSlider, onValueChange, LV_EVENT_VALUE_CHANGED, this);
  _riseLabel = lv_label_create(_screen);
  lv_obj_align_to(_riseLabel, _riseSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  snprintf(buf, sizeof(buf), "%d", s.FrostRise);
  lv_label_set_text(_riseLabel, buf);
}

void FrostScreen::onValueChange(lv_event_t* e) {
  static_cast<FrostScreen*>(lv_event_get_user_data(e))->saveSettings();
}

void FrostScreen::saveSettings() {
  auto s = NVstore.getUserSettings();
  s.FrostOn = lv_slider_get_value(_onSlider);
  s.FrostRise = lv_slider_get_value(_riseSlider);
  NVstore.setUserSettings(s);
  NVstore.save();

  char buf[16];
  if (s.FrostOn == 0) lv_label_set_text(_onLabel, "OFF");
  else { snprintf(buf, sizeof(buf), "%d", s.FrostOn); lv_label_set_text(_onLabel, buf); }
  snprintf(buf, sizeof(buf), "%d", s.FrostRise);
  lv_label_set_text(_riseLabel, buf);
}
