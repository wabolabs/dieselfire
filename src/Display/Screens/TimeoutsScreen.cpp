#include "TimeoutsScreen.h"
#include "../../Utility/NVStorage.h"

extern CHeaterStorage& NVstore;

TimeoutsScreen::TimeoutsScreen() : DieselScreen("Timeouts") {}

void TimeoutsScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Timeouts", LV_ALIGN_TOP_LEFT, 48, 3);

  auto& s = NVstore.getUserSettings();
  int y = 36;

  createLabel(_screen, "Dim after", LV_ALIGN_TOP_LEFT, 8, y);
  _dimSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_dimSlider, 80, y);
  lv_obj_set_size(_dimSlider, TFT_WIDTH - 130, 16);
  lv_slider_set_range(_dimSlider, 0, 300);
  lv_slider_set_value(_dimSlider, s.dimTime / 1000, LV_ANIM_OFF);
  lv_obj_add_event_cb(_dimSlider, onSliderChange, LV_EVENT_VALUE_CHANGED, this);
  _dimLabel = lv_label_create(_screen);
  lv_obj_align_to(_dimLabel, _dimSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  char buf[32];
  if (s.dimTime == 0) lv_label_set_text(_dimLabel, "Off");
  else { snprintf(buf, sizeof(buf), "%lds", s.dimTime / 1000); lv_label_set_text(_dimLabel, buf); }

  y += 40;
  createLabel(_screen, "Menu timeout", LV_ALIGN_TOP_LEFT, 8, y);
  _menuSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_menuSlider, 80, y);
  lv_obj_set_size(_menuSlider, TFT_WIDTH - 130, 16);
  lv_slider_set_range(_menuSlider, 0, 300);
  lv_slider_set_value(_menuSlider, s.menuTimeout / 1000, LV_ANIM_OFF);
  lv_obj_add_event_cb(_menuSlider, onSliderChange, LV_EVENT_VALUE_CHANGED, this);
  _menuLabel = lv_label_create(_screen);
  lv_obj_align_to(_menuLabel, _menuSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  if (s.menuTimeout == 0) lv_label_set_text(_menuLabel, "Off");
  else { snprintf(buf, sizeof(buf), "%lds", s.menuTimeout / 1000); lv_label_set_text(_menuLabel, buf); }
}

void TimeoutsScreen::onSliderChange(lv_event_t* e) {
  static_cast<TimeoutsScreen*>(lv_event_get_user_data(e))->save();
}

void TimeoutsScreen::save() {
  auto s = NVstore.getUserSettings();
  s.dimTime = lv_slider_get_value(_dimSlider) * 1000L;
  s.menuTimeout = lv_slider_get_value(_menuSlider) * 1000L;
  NVstore.setUserSettings(s);
  NVstore.save();

  char buf[32];
  if (s.dimTime == 0) lv_label_set_text(_dimLabel, "Off");
  else { snprintf(buf, sizeof(buf), "%lds", s.dimTime / 1000); lv_label_set_text(_dimLabel, buf); }
  if (s.menuTimeout == 0) lv_label_set_text(_menuLabel, "Off");
  else { snprintf(buf, sizeof(buf), "%lds", s.menuTimeout / 1000); lv_label_set_text(_menuLabel, buf); }
}
