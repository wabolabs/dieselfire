#include "heater_emu.h"
#include "vserial.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

HeaterEmulator::HeaterEmulator() {}
HeaterEmulator::~HeaterEmulator() {}

void HeaterEmulator::begin(VirtualSerial* serial) {
  _serial = serial;
}

// ── CRC-16 MODBUS ───────────────────────────────────────────
static uint16_t crc16_table(uint8_t v, uint16_t crc) {
  static const uint16_t table[256] = {
    0X0000,0XC0C1,0XC181,0X0140,0XC301,0X03C0,0X0280,0XC241,
    0XC601,0X06C0,0X0780,0XC741,0X0500,0XC5C1,0XC481,0X0440,
    0XCC01,0X0CC0,0X0D80,0XCD41,0X0F00,0XCFC1,0XCE81,0X0E40,
    0X0A00,0XCAC1,0XCB81,0X0B40,0XC901,0X09C0,0X0880,0XC841,
    0XD801,0X18C0,0X1980,0XD941,0X1B00,0XDBC1,0XDA81,0X1A40,
    0X1E00,0XDEC1,0XDF81,0X1F40,0XDD01,0X1DC0,0X1C80,0XDC41,
    0X1400,0XD4C1,0XD581,0X1540,0XD701,0X17C0,0X1680,0XD641,
    0XD201,0X12C0,0X1380,0XD341,0X1100,0XD1C1,0XD081,0X1040,
    0XF001,0X30C0,0X3180,0XF141,0X3300,0XF3C1,0XF281,0X3240,
    0X3600,0XF6C1,0XF781,0X3740,0XF501,0X35C0,0X3480,0XF441,
    0X3C00,0XFCC1,0XFD81,0X3D40,0XFF01,0X3FC0,0X3E80,0XFE41,
    0XFA01,0X3AC0,0X3B80,0XFB41,0X3900,0XF9C1,0XF881,0X3840,
    0X2800,0XE8C1,0XE981,0X2940,0XEB01,0X2BC0,0X2A80,0XEA41,
    0XEE01,0X2EC0,0X2F80,0XEF41,0X2D00,0XEDC1,0XEC81,0X2C40,
    0XE401,0X24C0,0X2580,0XE541,0X2700,0XE7C1,0XE681,0X2640,
    0X2200,0XE2C1,0XE381,0X2340,0XE101,0X21C0,0X2080,0XE041,
    0XA001,0X60C0,0X6180,0XA141,0X6300,0XA3C1,0XA281,0X6240,
    0X6600,0XA6C1,0XA781,0X6740,0XA501,0X65C0,0X6480,0XA441,
    0X6C00,0XACC1,0XAD81,0X6D40,0XAF01,0X6FC0,0X6E80,0XAE41,
    0XAA01,0X6AC0,0X6B80,0XAB41,0X6900,0XA9C1,0XA881,0X6840,
    0X7800,0XB8C1,0XB981,0X7940,0XBB01,0X7BC0,0X7A80,0XBA41,
    0XBE01,0X7EC0,0X7F80,0XBF41,0X7D00,0XBDC1,0XBC81,0X7C40,
    0XB401,0X74C0,0X7580,0XB541,0X7700,0XB7C1,0XB681,0X7640,
    0X7200,0XB2C1,0XB381,0X7340,0XB101,0X71C0,0X7080,0XB041,
    0X5000,0X90C1,0X9181,0X5140,0X9301,0X53C0,0X5280,0X9241,
    0X9601,0X56C0,0X5780,0X9741,0X5500,0X95C1,0X9481,0X5440,
    0X9C01,0X5CC0,0X5D80,0X9D41,0X5F00,0X9FC1,0X9E81,0X5E40,
    0X5A00,0X9AC1,0X9B81,0X5B40,0X9901,0X59C0,0X5880,0X9841,
    0X8801,0X48C0,0X4980,0X8941,0X4B00,0X8BC1,0X8A81,0X4A40,
    0X4E00,0X8EC1,0X8F81,0X4F40,0X8D01,0X4DC0,0X4C80,0X8C41,
    0X4400,0X84C1,0X8581,0X4540,0X8701,0X47C0,0X4680,0X8641,
    0X8201,0X42C0,0X4380,0X8341,0X4100,0X81C1,0X8081,0X4040,
  };
  return (crc >> 8) ^ table[(crc ^ v) & 0xFF];
}

uint16_t HeaterEmulator::crc16(const uint8_t* data, int len) {
  uint16_t crc = 0xFFFF;
  for (int i = 0; i < len; i++) crc = crc16_table(data[i], crc);
  return crc;
}

void HeaterEmulator::buildFrame(uint8_t* buf, uint8_t rs, uint8_t es, bool badCRC) {
  memset(buf, 0, 24);
  buf[0] = _faultPassive ? 0x78 : 0x76;
  buf[1] = 0x16;
  buf[2] = rs;
  buf[3] = es;

  uint16_t sv = _batteryV * 10;
  buf[4] = (sv >> 8) & 0xFF; buf[5] = sv & 0xFF;
  buf[6] = (_fanRPM >> 8) & 0xFF; buf[7] = _fanRPM & 0xFF;
  uint16_t fv = _fanVolts * 10;
  buf[8] = (fv >> 8) & 0xFF; buf[9] = fv & 0xFF;
  uint16_t hx = (_hxTemp > 0) ? (uint16_t)_hxTemp : 0;
  buf[10] = (hx >> 8) & 0xFF; buf[11] = hx & 0xFF;
  uint16_t gv = _glowVolts * 10;
  buf[12] = (gv >> 8) & 0xFF; buf[13] = gv & 0xFF;
  uint16_t gc = _glowAmps * 100;
  buf[14] = (gc >> 8) & 0xFF; buf[15] = gc & 0xFF;
  buf[16] = (uint8_t)(_pumpHz * 10);
  buf[17] = _storedError;
  buf[19] = buf[16];
  buf[20] = 0x64;

  uint16_t crc = crc16(buf, 22);
  if (badCRC) crc ^= 0xFFFF;
  buf[22] = (crc >> 8) & 0xFF; buf[23] = crc & 0xFF;
}

void HeaterEmulator::tick(int ms) {
  advanceState(ms);

  // Process incoming TX frames from BlueWire task
  while (_serial) {
    int avail = _serial->available();
    if (avail <= 0) break;
    int c = _serial->read();
    if (c < 0) break;
    if (_txBytes >= 24) { _txBytes = 0; continue; }
    _txBuf[_txBytes++] = (uint8_t)c;

    if (_txBytes == 24) {
      // Full frame received — process command
      uint8_t cmd = _txBuf[2];
      int8_t demand = (int8_t)_txBuf[4];
      if (demand > 0 && demand < 50) _demandDegC = demand;

      if (cmd == 0xA0 && _runState == OFF) {
        _runState = STARTING; _stateMs = 0; _errState = 0;
      } else if (cmd == 0x05 && (_runState == RUNNING || _runState == IGNITED || _runState == GLOWING)) {
        _runState = STOPPING; _stateMs = 0;
      }

      // Response with delay
      _responseDelay = _faultSlowMs;
      _txBytes = 0;

      if (!_faultNoResp) {
        uint8_t rxBuf[24];
        buildFrame(rxBuf, (uint8_t)_runState, _errState, _faultBadCRC);
        if (_faultPartial) {
          int sendBytes = (rand() % 12) + 6;
          _serial->write(rxBuf, sendBytes);
        } else {
          _serial->write(rxBuf, 24);
        }
        if (_faultRogue) {
          uint8_t rogue[4] = {0xAA, 0xBB, 0xCC, 0xDD};
          _serial->write(rogue, 4);
        }
      }
    }
  }
}

void HeaterEmulator::advanceState(int ms) {
  if (_responseDelay > 0) {
    _responseDelay -= ms;
    if (_responseDelay < 0) _responseDelay = 0;
  }

  _stateMs += ms;

  switch (_runState) {
    case OFF:
      _pumpHz = 0; _fanRPM = 0; _glowPower = 0;
      _glowVolts = 0; _glowAmps = 0; _fanVolts = 0;
      _batteryV = 12.6f; _stateMs = 0;
      break;

    case STARTING: {
      float t = (_stateMs % 3000) / 1000.0f;
      _pumpHz = 2.0f + t * 0.5f;
      _fanRPM = 600 + (int)(t * 200);
      _glowPower = 20.0f + t * 10.0f;
      _glowVolts = 11.0f; _glowAmps = _glowPower / _glowVolts;
      _fanVolts = 1.5f + t * 0.5f; _batteryV = 12.3f;
      if (_stateMs >= 3000) { _runState = GLOWING; _stateMs = 0; }
      break;
    }

    case GLOWING: {
      float t = (_stateMs % 12000) / 1000.0f;
      _pumpHz = 3.0f + t * 0.1f;
      _fanRPM = 1000 + (int)(t * 50);
      _glowPower = 70.0f; _glowVolts = 10.5f;
      _glowAmps = _glowPower / _glowVolts; _fanVolts = 2.5f;
      _batteryV = 12.1f;
      _hxTemp += 0.3f * (ms / 1000.0f);
      if (_hxTemp > 40) _hxTemp = 40;
      if (_stateMs >= 12000) { _runState = IGNITED; _stateMs = 0; }
      break;
    }

    case IGNITED: {
      float t = (_stateMs % 2000) / 1000.0f;
      _pumpHz = 4.0f + t * 0.2f;
      _fanRPM = 1600 + (int)(t * 200);
      _glowPower = 45.0f - t * 20.0f;
      _glowVolts = 11.0f; _glowAmps = _glowPower / _glowVolts;
      _fanVolts = 3.0f; _batteryV = 12.2f;
      _hxTemp += 1.5f * (ms / 1000.0f);
      if (_hxTemp > 55) _hxTemp = 55;
      if (_stateMs >= 2000) { _runState = RUNNING; _stateMs = 0; }
      break;
    }

    case RUNNING: {
      int runningTicks = _stateMs / 1000;
      _pumpHz = 3.0f + (_demandDegC - 8) * 0.1f;
      if (_pumpHz < 1.5f) _pumpHz = 1.5f;
      _fanRPM = 1800 + (int)(_pumpHz * 200) + (runningTicks % 5) * 20;
      _glowPower = 2.0f; _glowVolts = 0; _glowAmps = 0;
      _fanVolts = 3.5f; _batteryV = 12.8f;
      _hxTemp = 55.0f + (_demandDegC - 8) * 2.0f;
      if (_hxTemp > 80) _hxTemp = 80;
      _ambientC += 0.002f * ms;
      _fuelMl += _pumpHz * 0.001f * ms;
      break;
    }

    case STOPPING: {
      float t = (_stateMs % 3000) / 1000.0f;
      _pumpHz = 1.8f - t * 0.6f;
      if (_pumpHz < 0) _pumpHz = 0;
      _fanRPM = 2800 + (int)((1.0f - t) * 200);
      _glowPower = 0; _glowVolts = 0; _glowAmps = 0;
      _fanVolts = 4.0f - t * 0.5f; _batteryV = 12.5f;
      _hxTemp -= 2.0f * (ms / 1000.0f);
      if (_stateMs >= 3000) { _runState = SHUTTING_DOWN; _stateMs = 0; }
      break;
    }

    case SHUTTING_DOWN: {
      float t = (_stateMs % 5000) / 1000.0f;
      _pumpHz = 0; _fanRPM = 2000 - (int)(t * 400);
      if (_fanRPM < 0) _fanRPM = 0;
      _fanVolts = 3.0f - t * 0.6f;
      if (_fanVolts < 0) _fanVolts = 0;
      _batteryV = 12.4f;
      _hxTemp -= 1.0f * (ms / 1000.0f); _fuelMl += 0.001f * ms;
      if (_stateMs >= 5000) { _runState = COOLING; _stateMs = 0; }
      break;
    }

    case COOLING: {
      _fanRPM = 0; _fanVolts = 0; _batteryV = 12.6f;
      _hxTemp -= 0.8f * (ms / 1000.0f);
      if (_stateMs >= 8000) { _runState = OFF; _stateMs = 0; }
      break;
    }

    default: _runState = OFF; break;
  }
}
