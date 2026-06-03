#pragma once
// Heater emulator — cooperative tick() model.
// No threads. Call tick(deltaMs) from the main loop.

#include <cstdint>
#include <atomic>

class VirtualSerial;

class HeaterEmulator {
public:
  HeaterEmulator();
  ~HeaterEmulator();

  // Attach to a VirtualSerial (the BlueWire task's peer).
  void begin(VirtualSerial* serial);

  // Called from main loop, advances heater state by ms milliseconds.
  void tick(int ms);

  // ── Fault injection ────────────────────────────────────────
  void setBadCRC(bool on)       { _faultBadCRC = on; }
  void setPartialFrame(bool on) { _faultPartial = on; }
  void setNoResponse(bool on)   { _faultNoResp = on; }
  void setSlowResponse(int ms)  { _faultSlowMs = ms; }
  void setRogueBytes(bool on)   { _faultRogue = on; }
  void setPassiveMode(bool on)  { _faultPassive = on; }

  // ── State queries ──────────────────────────────────────────
  int   getRunState() const { return _runState; }
  int   getErrState() const { return _errState; }
  float getAmbientTemp() const { return _ambientC; }
  float getHeatExchangerTemp() const { return _hxTemp; }
  float getPumpHz() const { return _pumpHz; }
  int   getFanRPM() const { return _fanRPM; }
  float getGlowPower() const { return _glowPower; }
  float getBatteryVolts() const { return _batteryV; }
  float getFuelUsedMl() const { return _fuelMl; }

  // Fault injection getters
  bool getBadCRC() const       { return _faultBadCRC; }
  bool getPartialFrame() const { return _faultPartial; }
  bool getNoResponse() const   { return _faultNoResp; }
  bool getRogueBytes() const   { return _faultRogue; }
  bool getPassiveMode() const  { return _faultPassive; }

private:
  void buildFrame(uint8_t* buf, uint8_t rs, uint8_t es, bool badCRC);
  uint16_t crc16(const uint8_t* data, int len);
  void advanceState(int ms);
  void processIncoming();

  enum State : uint8_t {
    OFF = 0, STARTING = 1, GLOWING = 2, RETRY_PAUSE = 3,
    IGNITED = 4, RUNNING = 5, STOPPING = 6,
    SHUTTING_DOWN = 7, COOLING = 8,
  };
  State _runState = OFF;
  uint8_t _errState = 0;
  uint8_t _storedError = 0;

  float _ambientC = 10.0f, _hxTemp = 10.0f;
  float _pumpHz = 0.0f;
  int   _fanRPM = 0;
  float _fanVolts = 0.0f, _glowPower = 0.0f;
  float _glowVolts = 0.0f, _glowAmps = 0.0f;
  float _batteryV = 12.6f, _fuelMl = 0.0f;
  uint8_t _fanSensor = 1, _glowDrive = 5;
  int8_t _demandDegC = 20;
  int _stateMs = 0;

  // TX frame buffer (received from BlueWire task)
  uint8_t _txBuf[24] = {};
  int _txBytes = 0;
  int _responseDelay = 0;

  VirtualSerial* _serial = nullptr;

  bool _faultBadCRC = false, _faultPartial = false;
  bool _faultNoResp = false, _faultRogue = false;
  bool _faultPassive = false;
  int  _faultSlowMs = 5;
};
