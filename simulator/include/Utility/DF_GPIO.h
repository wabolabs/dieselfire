#pragma once
// Mock: Utility/DF_GPIO.h
#include <cstdint>
#include <cstdio>

class CGPIOin1 { public: enum Modes { Disabled, Start, Run, StartStop, Stop }; };
class CGPIOin2 { public: enum Modes { Disabled, Stop, Thermostat, FuelReset }; };

class CGPIOin {
public:
  void begin(int, int, int, int, int) {}
  void manage() {}
  uint8_t getState(int) { return 0; }
  CGPIOin1::Modes getMode1() const { return CGPIOin1::Disabled; }
  CGPIOin2::Modes getMode2() const { return CGPIOin2::Disabled; }
  void simulateKey(uint8_t) {}
};

class CGPIOoutBase { public: CGPIOoutBase() {} };
class CGPIOout1 : public CGPIOoutBase { public: enum Modes { Disabled, User, OnSts, Thresh }; };
class CGPIOout2 : public CGPIOoutBase { public: enum Modes { Disabled, User, Thresh, OnSts }; };

class CGPIOout {
public:
  void begin(int, int, int, int) {}
  void manage() {}
  void setThresh(int, int) {}
  uint8_t getState(int) { return 0; }
  void setState(int, bool) {}
  CGPIOout1::Modes getMode1() const { return CGPIOout1::Disabled; }
  CGPIOout2::Modes getMode2() const { return CGPIOout2::Disabled; }
};

class CGPIOalg {
public:
  enum Modes { Disabled };
  void begin(int, Modes) {}
  void manage() {}
  int getValue() { return 42; }
  Modes getMode() const { return Disabled; }
};

extern CGPIOin GPIOin;
extern CGPIOout GPIOout;
extern CGPIOalg GPIOalg;

extern const char* GPIOin1Names[];
extern const char* GPIOin2Names[];
extern const char* GPIOout1Names[];
extern const char* GPIOout2Names[];
extern const char* GPIOalgNames[];
