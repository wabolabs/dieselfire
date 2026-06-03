#pragma once

#include "DieselScreen.h"

class TimerEditScreen : public DieselScreen {
public:
  TimerEditScreen();
  void onLoad() override;

private:
  static void onTimerChange(lv_event_t* e);
  void loadTimer(int idx);
  void saveTimer();
  void updateLabels();
  void toggleDay(int day);

  int _timerIdx = 0;
  lv_obj_t* _timerSelector = nullptr;
  lv_obj_t* _startH = nullptr;
  lv_obj_t* _startM = nullptr;
  lv_obj_t* _stopH = nullptr;
  lv_obj_t* _stopM = nullptr;
  lv_obj_t* _dayBtns[7] = {};
  lv_obj_t* _repeatSwitch = nullptr;
  lv_obj_t* _tempSlider = nullptr;
  lv_obj_t* _tempLabel = nullptr;
};
