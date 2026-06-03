#pragma once
// Mock: Utility/NVStorage.h
#include <Arduino.h>
#include <cstdint>
#include <cstring>
#include "RTC/Timers.h"

struct sCyclicThermostat {
  int8_t Stop = 0;
  int8_t Start = -1;
  bool isEnabled() const { return Stop != 0; }
  bool valid() const { return true; }
};

struct sUserSettings {
  long dimTime = 60000;
  long menuTimeout = 60000;
  long ExtThermoTimeout = 0;
  uint8_t ThermostatMethod = 0;
  float ThermostatWindow = 1.0f;
  uint8_t FrostOn = 0;
  uint8_t FrostRise = 5;
  uint8_t useThermostat = 1;
  uint8_t humidityStart = 0;
  sCyclicThermostat cyclic;
  uint8_t degF = 0;
  uint16_t FrameRate = 1000;
  uint8_t enableOTA = 0;
  uint8_t wifiMode = 1;
  uint8_t menuMode = 0;
  uint8_t clock12hr = 0;
  uint8_t holdPassword = 0;
  uint8_t HomeMenu[3] = {};
  uint8_t GPIO[8] = {};
  uint8_t JSON[3] = {};
};

struct sHeaterTuning {
  uint8_t Pmin = 14;
  uint8_t Pmax = 45;
  uint16_t Fmin = 1500;
  uint16_t Fmax = 4500;
  uint8_t sysVoltage = 120;
  uint8_t fanSensor = 1;
  uint8_t glowDrive = 5;
  uint8_t lowVolts = 115;
  float pumpCal = 0.022f;
  uint16_t maxFuelUsage = 0;
  uint16_t warnFuelUsage = 0;
  float getPmin() const { return Pmin * 0.1f; }
  float getPmax() const { return Pmax * 0.1f; }
  void setPmin(float v) { Pmin = v * 10; }
  void setPmax(float v) { Pmax = v * 10; }
  void setFmin(int v) { Fmin = v; }
  void setFmax(int v) { Fmax = v; }
};

class CHeaterStorage {
public:
  sUserSettings _settings;
  sHeaterTuning _tuning;
  const sUserSettings& getUserSettings() const { return _settings; }
  void setUserSettings(const sUserSettings& s) { _settings = s; }
  sTimer _timers[14];
  void save() {}
  void doSave() {}
  void load() {}
  void init() {}
  void getTimerInfo(int idx, sTimer& t) const {
    if (idx >= 0 && idx < 14) t = _timers[idx];
  }
  void setTimerInfo(int idx, const sTimer& t) {
    if (idx >= 0 && idx < 14) _timers[idx] = t;
  }
  const sHeaterTuning& getHeaterTuning() const { return _tuning; }
  void setHeaterTuning(const sHeaterTuning& t) { _tuning = t; }
};

extern CHeaterStorage& NVstore;
