#pragma once
// Mock: Utility/NVStorage.h
#include <Arduino.h>
#include <cstdint>

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

class CHeaterStorage {
public:
  sUserSettings _settings;
  const sUserSettings& getUserSettings() const { return _settings; }
  void setUserSettings(const sUserSettings& s) { _settings = s; }
  void save() {}
  void doSave() {}
  void load() {}
  void init() {}
};

extern CHeaterStorage NVstore;
