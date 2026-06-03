#pragma once

#include "DieselScreen.h"

class HeaterSettingsScreen : public DieselScreen {
public:
  HeaterSettingsScreen();
  void onLoad() override;

private:
  static void onChange(lv_event_t* e);
  void save();
  lv_obj_t* _voltDropdown = nullptr;
  lv_obj_t* _fanDropdown = nullptr;
  lv_obj_t* _glowSlider = nullptr;
  lv_obj_t* _glowLabel = nullptr;
};
