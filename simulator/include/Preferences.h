#pragma once
// Mock: Preferences.h (ESP32 NVS storage)
#include <stdint.h>
#include <cstddef>

class Preferences {
public:
  bool begin(const char*, bool) { return true; }
  void end() {}
  void clear() {}
  bool putUChar(const char*, uint8_t) { return true; }
  bool putInt(const char*, int32_t) { return true; }
  bool putUInt(const char*, uint32_t) { return true; }
  bool putFloat(const char*, float) { return true; }
  bool putBool(const char*, bool) { return true; }
  bool putBytes(const char*, const void*, size_t) { return true; }
  uint8_t getUChar(const char*, uint8_t def) { return def; }
  int32_t getInt(const char*, int32_t def) { return def; }
  uint32_t getUInt(const char*, uint32_t def) { return def; }
  float getFloat(const char*, float def) { return def; }
  bool getBool(const char*, bool def) { return def; }
};
