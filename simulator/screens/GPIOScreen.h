#pragma once

#include "DieselScreen.h"

class GPIOScreen : public DieselScreen {
public:
  GPIOScreen();
  void onLoad() override;
  void onTimer() override;

private:
  void updateData();
  lv_obj_t* _in1Label = nullptr;
  lv_obj_t* _in2Label = nullptr;
  lv_obj_t* _out1Label = nullptr;
  lv_obj_t* _out2Label = nullptr;
  lv_obj_t* _analogLabel = nullptr;
};
