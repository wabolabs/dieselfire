#include "LVCScreen.h"
#include "../../Utility/NVStorage.h"
#include <cstdio>

extern CHeaterStorage& NVstore;

LVCScreen::LVCScreen() : DieselScreen("LVC") {}

void LVCScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Low Voltage Cutoff", LV_ALIGN_TOP_LEFT, 48, 3);

  auto& t = NVstore.getHeaterTuning();
  int y = 40;

  createLabel(_screen, "Shutdown <", LV_ALIGN_TOP_LEFT, 8, y);
  _lvcSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_lvcSlider, 90, y); lv_obj_set_size(_lvcSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_lvcSlider, 0, 130);
  lv_slider_set_value(_lvcSlider, t.lowVolts, LV_ANIM_OFF);
  lv_obj_add_event_cb(_lvcSlider, onChange, LV_EVENT_VALUE_CHANGED, this);
  _lvcLabel = lv_label_create(_screen);
  lv_obj_align_to(_lvcLabel, _lvcSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  char buf[16];
  if (t.lowVolts == 0) lv_label_set_text(_lvcLabel, "OFF");
  else { snprintf(buf, sizeof(buf), "%.1fV", t.lowVolts / 10.0f); lv_label_set_text(_lvcLabel, buf); }
}

void LVCScreen::onChange(lv_event_t* e) {
  static_cast<LVCScreen*>(lv_event_get_user_data(e))->save();
}

void LVCScreen::save() {
  auto t = NVstore.getHeaterTuning();
  t.lowVolts = lv_slider_get_value(_lvcSlider);
  NVstore.setHeaterTuning(t);
  NVstore.save();

  char buf[16];
  if (t.lowVolts == 0) lv_label_set_text(_lvcLabel, "OFF");
  else { snprintf(buf, sizeof(buf), "%.1fV", t.lowVolts / 10.0f); lv_label_set_text(_lvcLabel, buf); }
}
