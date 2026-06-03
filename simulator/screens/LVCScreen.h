#pragma once

#include "DieselScreen.h"

class LVCScreen : public DieselScreen {
public:
  LVCScreen();
  void onLoad() override;

private:
  static void onChange(lv_event_t* e);
  void save();
  lv_obj_t* _lvcSlider = nullptr;
  lv_obj_t* _lvcLabel = nullptr;
};
