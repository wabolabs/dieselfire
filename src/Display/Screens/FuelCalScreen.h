#pragma once

#include "DieselScreen.h"

class FuelCalScreen : public DieselScreen {
public:
  FuelCalScreen();
  void onLoad() override;

private:
  static void onChange(lv_event_t* e);
  void save();
  lv_obj_t* _calSlider = nullptr;
  lv_obj_t* _maxSlider = nullptr;
  lv_obj_t* _calLabel = nullptr;
  lv_obj_t* _maxLabel = nullptr;
};
