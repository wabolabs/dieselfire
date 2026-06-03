#pragma once

#include "DieselScreen.h"

class HumidityScreen : public DieselScreen {
public:
  HumidityScreen();
  void onLoad() override;

private:
  static void onSliderChange(lv_event_t* e);
  void save();

  lv_obj_t* _slider = nullptr;
  lv_obj_t* _label = nullptr;
};
