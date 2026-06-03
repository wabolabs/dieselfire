#pragma once
// Virtual serial port using Unix socket pair.
// Matches the HardwareSerial interface so BlueWireTask can use it directly.

#include <cstdint>
#include <cstddef>

class VirtualSerial {
public:
  VirtualSerial();
  ~VirtualSerial();

  // Create a socket pair. Call on both ends:
  //   int fds[2]; socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  //   blueWireSerial.begin(fds[0]);  // end for BlueWire task
  //   heaterEmu.begin(fds[1]);       // end for heater emulator
  void begin(int fd, bool nonBlocking = true);

  // HardwareSerial-compatible API
  int available();
  int read();
  size_t write(uint8_t c);
  size_t write(const uint8_t* buf, size_t len);
  void flush();

  // Close the serial
  void end();

  int fd() const { return _fd; }

private:
  int _fd = -1;
};
