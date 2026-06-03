#include "FuelCalScreen.h"
#include "../../Utility/NVStorage.h"
#include <cstdio>

extern CHeaterStorage& NVstore;

FuelCalScreen::FuelCalScreen() : DieselScreen("FuelCal") {}

void FuelCalScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Fuel Calibration", LV_ALIGN_TOP_LEFT, 48, 3);

  auto& t = NVstore.getHeaterTuning();
  int y = 36;

  createLabel(_screen, "mL/stroke", LV_ALIGN_TOP_LEFT, 8, y);
  _calSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_calSlider, 90, y); lv_obj_set_size(_calSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_calSlider, 5, 100);
  lv_slider_set_value(_calSlider, t.pumpCal * 1000, LV_ANIM_OFF);
  lv_obj_add_event_cb(_calSlider, onChange, LV_EVENT_VALUE_CHANGED, this);
  _calLabel = lv_label_create(_screen);
  lv_obj_align_to(_calLabel, _calSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  char buf[16]; snprintf(buf, sizeof(buf), "%.3f", t.pumpCal); lv_label_set_text(_calLabel, buf);
  y += 36;

  createLabel(_screen, "Max Tank", LV_ALIGN_TOP_LEFT, 8, y);
  _maxSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_maxSlider, 90, y); lv_obj_set_size(_maxSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_maxSlider, 0, 500);
  lv_slider_set_value(_maxSlider, t.maxFuelUsage, LV_ANIM_OFF);
  lv_obj_add_event_cb(_maxSlider, onChange, LV_EVENT_VALUE_CHANGED, this);
  _maxLabel = lv_label_create(_screen);
  lv_obj_align_to(_maxLabel, _maxSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  if (t.maxFuelUsage == 0) lv_label_set_text(_maxLabel, "Off");
  else { snprintf(buf, sizeof(buf), "%.1fL", t.maxFuelUsage / 10.0f); lv_label_set_text(_maxLabel, buf); }
}

void FuelCalScreen::onChange(lv_event_t* e) {
  static_cast<FuelCalScreen*>(lv_event_get_user_data(e))->save();
}

void FuelCalScreen::save() {
  auto t = NVstore.getHeaterTuning();
  t.pumpCal = lv_slider_get_value(_calSlider) / 1000.0f;
  t.maxFuelUsage = lv_slider_get_value(_maxSlider);
  NVstore.setHeaterTuning(t);
  NVstore.save();

  char buf[16];
  snprintf(buf, sizeof(buf), "%.3f", t.pumpCal); lv_label_set_text(_calLabel, buf);
  if (t.maxFuelUsage == 0) lv_label_set_text(_maxLabel, "Off");
  else { snprintf(buf, sizeof(buf), "%.1fL", t.maxFuelUsage / 10.0f); lv_label_set_text(_maxLabel, buf); }
}
