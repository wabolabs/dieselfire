#pragma once
// Mock: Protocol/Protocol.h
#include <cstdint>
#include <cstdio>

class CProtocol {
public:
  uint8_t Data[24] = {};
  uint8_t getRunState() const { return Data[0]; }
  uint8_t getErrState() const { return 0; }
  uint16_t getFan_Actual() const { return 2200; }
  float getPump_Actual() const { return 3.8f; }
  void setPump_Actual(float f) {}
  float getPump_Fixed() const { return 4.0f; }
  float getFan_Voltage() const { return 2.5f; }
  float getGlowPlug_Current() const { return 0.2f; }
  float getGlowPlug_Voltage() const { return 25.0f; }
  float getGlowPlug_Power() const { return 5.0f; }
  float getTemperature_HeatExchg() const { return 65.0f; }
  int getFan_Sensor() const { return 1; }
  int getGlowDrive() const { return 5; }
  float getSystemVoltage() const { return 12.0f; }
  float getVoltage_Supply() const { return 12.6f; }
  float getPump_Min() const { return 1.4f; }
  float getPump_Max() const { return 4.5f; }
  uint16_t getFan_Min() const { return 1500; }
  uint16_t getFan_Max() const { return 4500; }
  int8_t getHeaterDemand() const { return 20; }
  bool isThermostat() const { return true; }
  int getTemperature_Min() const { return 8; }
  int getTemperature_Max() const { return 35; }
  int getAltitude() const { return 50; }
  int getRunStateEx() const { return Data[0]; }
  const char* getRunStateStr() const { return "Running"; }
};

class CProtocolPackage {
public:
  CProtocol Heater;
  CProtocol Controller;
  void set(const CProtocol& h, const CProtocol& c) { Heater = h; Controller = c; }
  int getRunState() const { return Heater.getRunState(); }
  int getErrState() const { return Heater.getErrState(); }
  int getRunStateEx() const { return Heater.getRunStateEx(); }
  const char* getRunStateStr() const { return Heater.getRunStateStr(); }
  float getBattVoltage() const { return Heater.getVoltage_Supply(); }
  float getHeaterDemand() const { return Heater.getHeaterDemand(); }
  float getPump_Actual() const { return Heater.getPump_Actual(); }
  float getPump_Fixed() const { return Heater.getPump_Fixed(); }
  float getPump_Min() const { return Heater.getPump_Min(); }
  float getPump_Max() const { return Heater.getPump_Max(); }
  float getFan_Actual() const { return Heater.getFan_Actual(); }
  uint16_t getFan_Min() const { return Heater.getFan_Min(); }
  uint16_t getFan_Max() const { return Heater.getFan_Max(); }
  float getFan_Voltage() const { return Heater.getFan_Voltage(); }
  int getFan_Sensor() const { return Heater.getFan_Sensor(); }
  float getGlowPlug_Power() const { return Heater.getGlowPlug_Power(); }
  float getGlow_Voltage() const { return Heater.getGlowPlug_Voltage(); }
  float getGlow_Current() const { return Heater.getGlowPlug_Current(); }
  float getSystemVoltage() const { return Heater.getSystemVoltage(); }
  int getGlow_Drive() const { return Heater.getGlowDrive(); }
  int getAltitude() const { return Controller.getAltitude(); }
  float getTemperature_HeatExchg() const { return Heater.getTemperature_HeatExchg(); }
};

extern const CProtocolPackage& getHeaterInfo();
