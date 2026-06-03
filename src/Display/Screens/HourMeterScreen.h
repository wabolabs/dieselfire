#pragma once

#include "DieselScreen.h"

class HourMeterScreen : public DieselScreen {
public:
  HourMeterScreen();
  void onLoad() override;

private:
  lv_obj_t* _runLabel = nullptr;
  lv_obj_t* _glowLabel = nullptr;
  lv_obj_t* _upLabel = nullptr;
};
