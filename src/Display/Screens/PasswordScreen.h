#pragma once

#include "DieselScreen.h"

class PasswordScreen : public DieselScreen {
public:
  PasswordScreen(void (*onSuccess)(), DieselScreen* next);
  void onLoad() override;

private:
  static void onDigitClick(lv_event_t* e);
  void checkPassword();
  int _digits[4] = {};
  int _pos = 0;
  lv_obj_t* _dots[4] = {};
  lv_obj_t* _btnGrid = nullptr;
  void (*_onSuccess)() = nullptr;
  DieselScreen* _nextScreen = nullptr;
};
