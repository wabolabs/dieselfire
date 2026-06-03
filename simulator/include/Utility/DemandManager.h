#pragma once
// Mock: Utility/DemandManager.h
class CDemandManager {
public:
  static float getDegC() { return 20.0f; }
  static bool isThermostat() { return true; }
  static bool isExtThermostatMode() { return false; }
  static float getDemand() { return 20.0f; }
  static int getExtThermostatHoldTime() { return 0; }
  static void setThermostatMode(bool) {}
  static void setDegFMode(bool) {}
  static void deltaDemand(int) {}
  static void reload() {}
};
