#pragma once
// Mock: Utility/MQCOSensor.h

#include <cstdint>

class CMQCOSensor {
public:
  CMQCOSensor() {}
  void begin() {}
  void read() {}
  float getPPM() const { return 3.2f; }
  int getRawADC() const { return 512; }
  float getPinVoltage() const { return 0.4f; }
  float getSensorVoltage() const { return 0.8f; }
  bool getAlarmState() const { return false; }
  float getRatio() const { return 0.85f; }
  void calibrate() {}
  float getR0() const { return 10.0f; }
  void setR0(float) {}
  bool isReady() const { return true; }
};
