#pragma once

#include "DieselScreen.h"

class FuelMixtureScreen : public DieselScreen {
public:
  FuelMixtureScreen();
  void onLoad() override;

private:
  static void onChange(lv_event_t* e);
  void save();

  lv_obj_t* _pminSlider = nullptr;
  lv_obj_t* _pmaxSlider = nullptr;
  lv_obj_t* _fminSlider = nullptr;
  lv_obj_t* _fmaxSlider = nullptr;
  lv_obj_t* _pminLabel = nullptr;
  lv_obj_t* _pmaxLabel = nullptr;
  lv_obj_t* _fminLabel = nullptr;
  lv_obj_t* _fmaxLabel = nullptr;
};
