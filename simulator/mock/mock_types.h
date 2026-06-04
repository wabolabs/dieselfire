#ifndef MOCK_TYPES_H
#define MOCK_TYPES_H

#include <cstdint>
#include <cstdio>
#include <cstring>

// ── CProtocol frame data ────────────────────────────────────
class CProtocol {
public:
  uint8_t Data[24] = {};
  struct ControllerPart {
    uint8_t Byte0 = 0x76;
    uint8_t Command = 0;
    int8_t DesiredDemand = 20;
    uint8_t MinPumpFreq = 14;
    uint8_t MaxPumpFreq = 45;
    uint8_t MinFanRPM_MSB = 0;
    uint8_t MinFanRPM_LSB = 150;
    uint8_t FanSensor = 1;
    uint8_t Mode = 0x32;
    uint8_t GlowDrive = 5;
  } Controller;

  uint8_t getRunState() const { return Data[0]; }
  uint8_t getErrState() const { return 0; }
  uint16_t getFan_Actual() const { return 2200; }
  float getPump_Actual() const { return 3.8f; }
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

// ── CProtocolPackage ────────────────────────────────────────
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

// ── NVstore types ───────────────────────────────────────────
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
};

// ── Fuel gauge ──────────────────────────────────────────────
class CFuelGauge {
  float _used = 0.0f;
public:
  void init(float v) {}
  float Used_mL() const { return _used; }
  void reset() { _used = 0; }
  void Integrate(float freq) { _used += freq * 0.001f; }
};

// ── Filtered data ───────────────────────────────────────────
class CExpMean {
  float _val = 10.0f;
public:
  void reset(float v) { _val = v; }
  void update(float v) { _val = _val * 0.7f + v * 0.3f; }
  float getValue() const { return _val; }
  void setRounding(float r) {}
};

struct sFilteredData {
  CExpMean AmbientTemp;
  CExpMean ipVolts;
  CExpMean GlowAmps;
  CExpMean GlowVolts;
  CExpMean Fan;
  CExpMean FastipVolts;
  CExpMean FastGlowAmps;
};

// ── Demand manager ──────────────────────────────────────────
class CDemandManager {
public:
  enum eStartCode { eStartOK=0, eStartTooWarm=-1, eStartSuspend=-2, eStartLVC=-3, eStartLowFuel=-4 };
  static float getDegC() { return _degC; }
  static bool isThermostat() { return true; }
  static bool isExtThermostatMode() { return false; }
  static float getDemand() { return _degC; }
  static int getExtThermostatHoldTime() { return 0; }
  static void setThermostatMode(bool on) {}
  static void setDegFMode(bool on) {}
  static void deltaDemand(int d) { _degC += d; if (_degC < 8) _degC = 8; if (_degC > 35) _degC = 35; }
  static void reload() {}
  static uint8_t _degC;
};
uint8_t CDemandManager::_degC = 20;

// ── Debug port ──────────────────────────────────────────────
class DebugPortClass {
public:
  template<typename T> void print(T v) {}
  template<typename T> void println(T v) {}
  void printf(const char* fmt, ...) {}
  void begin(int baud) {}
  void handle() {}
  void setWelcomeMsg(const char*) {}
  void setBufferSize(int s) {}
};
extern DebugPortClass DebugPort;

// ── Global externs ──────────────────────────────────────────
extern CHeaterStorage NVstore;
extern CFuelGauge FuelGauge;
extern sFilteredData FilteredSamples;
extern CProtocolPackage BlueWireData;
extern const CProtocolPackage& getHeaterInfo();

#endif
