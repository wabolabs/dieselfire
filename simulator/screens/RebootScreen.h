#pragma once

#include "DieselScreen.h"

class RebootScreen : public DieselScreen {
public:
  RebootScreen();
  void onLoad() override;
  void onTimer() override;

private:
  int _countdown = 10;
  lv_obj_t* _label = nullptr;
};
