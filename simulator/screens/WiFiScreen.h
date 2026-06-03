#pragma once

#include "DieselScreen.h"

class WiFiScreen : public DieselScreen {
public:
  WiFiScreen();
  void onLoad() override;
  void onTimer() override;

private:
  void updateData();
  lv_obj_t* _modeLabel = nullptr;
  lv_obj_t* _ipLabel = nullptr;
  lv_obj_t* _rssiLabel = nullptr;
};
