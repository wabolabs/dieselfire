#pragma once

#include "DieselScreen.h"

class MQTTScreen : public DieselScreen {
public:
  MQTTScreen();
  void onLoad() override;
  void onTimer() override;

private:
  lv_obj_t* _valStatus = nullptr;
  lv_obj_t* _valHost = nullptr;
  lv_obj_t* _valPrefix = nullptr;
  lv_obj_t* _valHA = nullptr;
};
