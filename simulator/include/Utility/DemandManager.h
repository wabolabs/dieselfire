#pragma once
// Mock: Utility/DemandManager.h

#ifndef __DEMANDMANAGER_H__
#define __DEMANDMANAGER_H__

#include <cstdint>

class CDemandManager {
public:
  enum eStartCode { eStartOK=0, eStartTooWarm=-1, eStartSuspend=-2, eStartLVC=-3, eStartLowFuel=-4 };
  static float getDegC() { return _degC; }
  static bool isThermostat() { return true; }
  static bool isExtThermostatMode() { return false; }
  static float getDemand() { return _degC; }
  static int getExtThermostatHoldTime() { return 0; }
  static void setThermostatMode(bool) {}
  static void setDegFMode(bool) {}
  static void deltaDemand(int d) { _degC += d; if (_degC < 8) _degC = 8; if (_degC > 35) _degC = 35; }
  static void reload() {}
  static uint8_t _degC;
};

#endif
