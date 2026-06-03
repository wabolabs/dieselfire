#pragma once

#include "DieselScreen.h"

class FrostScreen : public DieselScreen {
public:
  FrostScreen();
  void onLoad() override;

private:
  static void onValueChange(lv_event_t* e);
  void saveSettings();

  lv_obj_t* _onSlider = nullptr;
  lv_obj_t* _onLabel = nullptr;
  lv_obj_t* _riseSlider = nullptr;
  lv_obj_t* _riseLabel = nullptr;
};
