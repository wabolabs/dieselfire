#pragma once
// Mock: Print.h
#include <stdint.h>
#include <stddef.h>

class Print {
public:
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t*, size_t s) { return s; }
  void print(const char*) {}
  void println(const char*) {}
  void print(int) {}
  void println(int) {}
  void print(float) {}
  void println(float) {}
  void print(double) {}
  void println(double) {}
  void printf(const char*, ...) {}
};
