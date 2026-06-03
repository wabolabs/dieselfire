#pragma once

#include "DieselScreen.h"

class SystemScreen : public DieselScreen {
public:
  SystemScreen();
  void onLoad() override;
  void onTimer() override;

private:
  void updateData();
  lv_obj_t* _verLabel = nullptr;
  lv_obj_t* _boardLabel = nullptr;
};
