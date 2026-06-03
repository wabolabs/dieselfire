#include "FuelMixtureScreen.h"
#include "../../Utility/NVStorage.h"
#include <cstdio>

extern CHeaterStorage& NVstore;

FuelMixtureScreen::FuelMixtureScreen() : DieselScreen("FuelMix") {}

void FuelMixtureScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Fuel Mixture", LV_ALIGN_TOP_LEFT, 48, 3);

  auto& t = NVstore.getHeaterTuning();
  int y = 36;

  createLabel(_screen, "Pump Min", LV_ALIGN_TOP_LEFT, 8, y);
  _pminSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_pminSlider, 90, y); lv_obj_set_size(_pminSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_pminSlider, 50, 500);
  lv_slider_set_value(_pminSlider, t.getPmin() * 10, LV_ANIM_OFF);
  lv_obj_add_event_cb(_pminSlider, onChange, LV_EVENT_VALUE_CHANGED, this);
  _pminLabel = lv_label_create(_screen);
  lv_obj_align_to(_pminLabel, _pminSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  char buf[16]; snprintf(buf, sizeof(buf), "%.1f", t.getPmin()); lv_label_set_text(_pminLabel, buf);
  y += 36;

  createLabel(_screen, "Pump Max", LV_ALIGN_TOP_LEFT, 8, y);
  _pmaxSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_pmaxSlider, 90, y); lv_obj_set_size(_pmaxSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_pmaxSlider, 100, 1000);
  lv_slider_set_value(_pmaxSlider, t.getPmax() * 10, LV_ANIM_OFF);
  lv_obj_add_event_cb(_pmaxSlider, onChange, LV_EVENT_VALUE_CHANGED, this);
  _pmaxLabel = lv_label_create(_screen);
  lv_obj_align_to(_pmaxLabel, _pmaxSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  snprintf(buf, sizeof(buf), "%.1f", t.getPmax()); lv_label_set_text(_pmaxLabel, buf);
  y += 36;

  createLabel(_screen, "Fan Min", LV_ALIGN_TOP_LEFT, 8, y);
  _fminSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_fminSlider, 90, y); lv_obj_set_size(_fminSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_fminSlider, 50, 300);
  lv_slider_set_value(_fminSlider, t.Fmin / 10, LV_ANIM_OFF);
  lv_obj_add_event_cb(_fminSlider, onChange, LV_EVENT_VALUE_CHANGED, this);
  _fminLabel = lv_label_create(_screen);
  lv_obj_align_to(_fminLabel, _fminSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  snprintf(buf, sizeof(buf), "%d", t.Fmin); lv_label_set_text(_fminLabel, buf);
  y += 36;

  createLabel(_screen, "Fan Max", LV_ALIGN_TOP_LEFT, 8, y);
  _fmaxSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_fmaxSlider, 90, y); lv_obj_set_size(_fmaxSlider, TFT_WIDTH - 140, 16);
  lv_slider_set_range(_fmaxSlider, 100, 600);
  lv_slider_set_value(_fmaxSlider, t.Fmax / 10, LV_ANIM_OFF);
  lv_obj_add_event_cb(_fmaxSlider, onChange, LV_EVENT_VALUE_CHANGED, this);
  _fmaxLabel = lv_label_create(_screen);
  lv_obj_align_to(_fmaxLabel, _fmaxSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
  snprintf(buf, sizeof(buf), "%d", t.Fmax); lv_label_set_text(_fmaxLabel, buf);
}

void FuelMixtureScreen::onChange(lv_event_t* e) {
  static_cast<FuelMixtureScreen*>(lv_event_get_user_data(e))->save();
}

void FuelMixtureScreen::save() {
  auto t = NVstore.getHeaterTuning();
  t.setPmin(lv_slider_get_value(_pminSlider) / 10.0f);
  t.setPmax(lv_slider_get_value(_pmaxSlider) / 10.0f);
  t.setFmin(lv_slider_get_value(_fminSlider) * 10);
  t.setFmax(lv_slider_get_value(_fmaxSlider) * 10);
  NVstore.setHeaterTuning(t);
  NVstore.save();

  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", t.getPmin()); lv_label_set_text(_pminLabel, buf);
  snprintf(buf, sizeof(buf), "%.1f", t.getPmax()); lv_label_set_text(_pmaxLabel, buf);
  snprintf(buf, sizeof(buf), "%d", t.Fmin); lv_label_set_text(_fminLabel, buf);
  snprintf(buf, sizeof(buf), "%d", t.Fmax); lv_label_set_text(_fmaxLabel, buf);
}
