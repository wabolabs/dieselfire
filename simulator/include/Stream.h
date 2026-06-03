#pragma once
// Mock: Stream.h
#include "Print.h"

class Stream : public Print {
public:
  virtual void flush() {}
  int available() { return 0; }
  int read() { return -1; }
  int peek() { return -1; }
  size_t readBytes(char*, size_t s) { return 0; }
  void setTimeout(unsigned long) {}
};
