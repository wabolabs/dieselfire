#include "vserial.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

VirtualSerial::VirtualSerial() {}

VirtualSerial::~VirtualSerial() { end(); }

void VirtualSerial::begin(int fd, bool nonBlocking) {
  _fd = fd;
  if (nonBlocking && _fd >= 0) {
    int flags = fcntl(_fd, F_GETFL, 0);
    fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
  }
}

int VirtualSerial::available() {
  if (_fd < 0) return 0;
  int count = 0;
  if (ioctl(_fd, FIONREAD, &count) < 0) return 0;
  return count;
}

int VirtualSerial::read() {
  if (_fd < 0) return -1;
  uint8_t c;
  ssize_t n = ::read(_fd, &c, 1);
  return (n == 1) ? c : -1;
}

size_t VirtualSerial::write(uint8_t c) {
  return write(&c, 1);
}

size_t VirtualSerial::write(const uint8_t* buf, size_t len) {
  if (_fd < 0) return 0;
  ssize_t n = ::write(_fd, buf, len);
  return (n > 0) ? (size_t)n : 0;
}

void VirtualSerial::flush() {
  fsync(_fd);
}

void VirtualSerial::end() {
  if (_fd >= 0) {
    ::close(_fd);
    _fd = -1;
  }
}
