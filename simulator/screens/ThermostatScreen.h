#pragma once

#include "DieselScreen.h"

class ThermostatScreen : public DieselScreen {
public:
  ThermostatScreen();
  void onLoad() override;

private:
  static void onValueChange(lv_event_t* e);
  void saveSettings();

  lv_obj_t* _methodDropdown = nullptr;
  lv_obj_t* _windowSlider = nullptr;
  lv_obj_t* _windowLabel = nullptr;
  lv_obj_t* _stopSlider = nullptr;
  lv_obj_t* _stopLabel = nullptr;
  lv_obj_t* _startSlider = nullptr;
  lv_obj_t* _startLabel = nullptr;
};
