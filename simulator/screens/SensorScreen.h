#pragma once

#include "DieselScreen.h"

class SensorScreen : public DieselScreen {
public:
  SensorScreen();
  void onLoad() override;
  void onTimer() override;

private:
  void updateData();
  lv_obj_t* _tempLabel = nullptr;
  lv_obj_t* _humLabel = nullptr;
  lv_obj_t* _altLabel = nullptr;
  lv_obj_t* _extTempLabel = nullptr;
};
