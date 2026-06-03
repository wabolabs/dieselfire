#pragma once
// Heater emulator for Blue Wire protocol testing.
// Runs in its own thread, reads TX frames from the BlueWire task,
// maintains a realistic heater state machine, and responds with
// CRC-correct RX frames.

#include <cstdint>
#include <thread>
#include <atomic>

class HeaterEmulator {
public:
  HeaterEmulator();
  ~HeaterEmulator();

  // Attach to one end of a socket pair (the other end is the BlueWire task)
  void begin(int fd);

  // Run loop (call from a thread, or call run() once per frame)
  void run();
  void stop();

  // ── Fault injection ────────────────────────────────────────
  void setBadCRC(bool on)       { _faultBadCRC = on; }
  void setPartialFrame(bool on) { _faultPartial = on; }
  void setNoResponse(bool on)   { _faultNoResp = on; }
  void setSlowResponse(int ms)  { _faultSlowMs = ms; }
  void setRogueBytes(bool on)   { _faultRogue = on; }
  void setPassiveMode(bool on)  { _faultPassive = on; }

  // ── State queries ──────────────────────────────────────────
  int  getRunState() const { return _runState; }
  int  getErrState() const { return _errState; }
  float getAmbientTemp() const { return _ambientC; }
  float getHeatExchangerTemp() const { return _hxTemp; }
  float getPumpHz() const { return _pumpHz; }
  int  getFanRPM() const { return _fanRPM; }
  float getGlowPower() const { return _glowPower; }
  float getBatteryVolts() const { return _batteryV; }
  float getFuelUsedMl() const { return _fuelMl; }

  // ── Fault injection getters ────────────────────────────────
  bool getBadCRC() const       { return _faultBadCRC; }
  bool getPartialFrame() const { return _faultPartial; }
  bool getNoResponse() const   { return _faultNoResp; }
  bool getRogueBytes() const   { return _faultRogue; }
  bool getPassiveMode() const  { return _faultPassive; }

private:
  void buildFrame(uint8_t* buf, uint8_t rs, uint8_t es, bool badCRC);
  uint16_t crc16(const uint8_t* data, int len);
  void advanceState(int ms);

  // Heater state machine
  enum State : uint8_t {
    OFF = 0,
    STARTING = 1,
    GLOWING = 2,
    RETRY_PAUSE = 3,
    IGNITED = 4,
    RUNNING = 5,
    STOPPING = 6,
    SHUTTING_DOWN = 7,
    COOLING = 8,
  };
  State _runState = OFF;
  uint8_t _errState = 0;
  uint8_t _storedError = 0;

  // Telemetry
  float _ambientC = 10.0f;
  float _hxTemp = 10.0f;
  float _pumpHz = 0.0f;
  int   _fanRPM = 0;
  float _fanVolts = 0.0f;
  float _glowPower = 0.0f;
  float _glowVolts = 0.0f;
  float _glowAmps = 0.0f;
  float _batteryV = 12.6f;
  float _fuelMl = 0.0f;
  uint8_t _fanSensor = 1;
  uint8_t _glowDrive = 5;

  // Demand tracking
  int8_t _demandDegC = 20;

  // State timing
  int _stateMs = 0;

  // File descriptor
  int _fd = -1;
  std::thread _thread;
  std::atomic<bool> _running{false};

  // Fault injection
  std::atomic<bool>  _faultBadCRC{false};
  std::atomic<bool>  _faultPartial{false};
  std::atomic<bool>  _faultNoResp{false};
  std::atomic<int>   _faultSlowMs{5};
  std::atomic<bool>  _faultRogue{false};
  std::atomic<bool>  _faultPassive{false};
};
