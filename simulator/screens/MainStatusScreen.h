#pragma once

#include "DieselScreen.h"

class MainStatusScreen : public DieselScreen {
public:
  MainStatusScreen();
  void onLoad() override;
  void onTimer() override;
  void onSettings() override;

private:
  void buildTelemetryTab(lv_obj_t* parent);
  void buildBigTempTab(lv_obj_t* parent);
  void buildClockTab(lv_obj_t* parent);
  void buildPrimingTab(lv_obj_t* parent);

  void updateTelemetry();
  void updateBigTemp();
  void updateClockTabContent();
  void updatePriming();
  void updateHeaterButton();

  static void onHeaterBtn(lv_event_t* e);
  static void onTempUp(lv_event_t* e);
  static void onTempDown(lv_event_t* e);
  static void onPrimeBtn(lv_event_t* e);
  static void onResetFuel(lv_event_t* e);

  lv_obj_t* _tabview = nullptr;
  lv_obj_t* _telemTemp = nullptr;
  lv_obj_t* _telemRpm = nullptr;
  lv_obj_t* _telemPump = nullptr;
  lv_obj_t* _telemGlow = nullptr;
  lv_obj_t* _telemFuel = nullptr;
  lv_obj_t* _telemState = nullptr;
  lv_obj_t* _bigTemp = nullptr;
  lv_obj_t* _bigState = nullptr;
  lv_obj_t* _clockDisplay = nullptr;
  lv_obj_t* _clockDate = nullptr;
  lv_obj_t* _primeStatus = nullptr;
  lv_obj_t* _primeFuelUsed = nullptr;

  lv_obj_t* _heaterBtn = nullptr;
  lv_obj_t* _tempUp = nullptr;
  lv_obj_t* _tempDown = nullptr;
  lv_obj_t* _primeBtn = nullptr;
  lv_obj_t* _resetFuelBtn = nullptr;
};
