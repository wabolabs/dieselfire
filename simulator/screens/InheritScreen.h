#pragma once

#include "DieselScreen.h"

class InheritScreen : public DieselScreen {
public:
  InheritScreen();
  void onLoad() override;

private:
  lv_obj_t* _statusLabel = nullptr;
};
