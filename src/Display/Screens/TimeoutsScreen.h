#pragma once

#include "DieselScreen.h"

class TimeoutsScreen : public DieselScreen {
public:
  TimeoutsScreen();
  void onLoad() override;

private:
  static void onSliderChange(lv_event_t* e);
  void save();

  lv_obj_t* _dimSlider = nullptr;
  lv_obj_t* _dimLabel = nullptr;
  lv_obj_t* _menuSlider = nullptr;
  lv_obj_t* _menuLabel = nullptr;
};
