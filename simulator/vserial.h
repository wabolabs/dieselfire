#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdlib>

class VirtualSerial {
public:
  VirtualSerial();
  ~VirtualSerial();
  VirtualSerial(VirtualSerial&&) = delete;
  VirtualSerial(const VirtualSerial&) = delete;

  void connect(VirtualSerial* peer);

  int available();
  int read();
  size_t write(uint8_t c);
  size_t write(const uint8_t* buf, size_t len);
  void flush();
  void end();

private:
  // Ring buffer: dynamically allocated to guarantee proper init
  uint8_t* _buf;
  size_t _cap, _pos, _count;

  // Pointer to the peer's buffer (where we read from)
  VirtualSerial* _peer;

  friend class VirtualSerialPair;
};

struct VirtualSerialPair {
  VirtualSerial a;
  VirtualSerial b;
  VirtualSerialPair() { a.connect(&b); b.connect(&a); }
  VirtualSerialPair(const VirtualSerialPair&) = delete;
  void reset() { a._count = 0; a._pos = 0; b._count = 0; b._pos = 0; a._peer = &b; b._peer = &a; }
};
