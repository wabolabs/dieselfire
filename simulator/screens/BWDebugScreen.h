#pragma once

#include "DieselScreen.h"

class BWDebugScreen : public DieselScreen {
public:
  BWDebugScreen();
  void onLoad() override;
  void onTimer() override;

private:
  lv_obj_t* _stateLabel = nullptr;
  lv_obj_t* _txHex = nullptr;
  lv_obj_t* _rxHex = nullptr;
  lv_obj_t* _heaterState = nullptr;
  lv_obj_t* _heaterTemp = nullptr;
  lv_obj_t* _heaterPump = nullptr;
  lv_obj_t* _heaterFan = nullptr;
  lv_obj_t* _faultLabel = nullptr;

  lv_obj_t* _btnBadCRC = nullptr;
  lv_obj_t* _btnPartial = nullptr;
  lv_obj_t* _btnNoResp = nullptr;
  lv_obj_t* _btnRogue = nullptr;
  lv_obj_t* _btnPassive = nullptr;

  static void onBtnClick(lv_event_t* e);
};
