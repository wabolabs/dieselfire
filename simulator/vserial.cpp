#include "vserial.h"
#include <cstring>

VirtualSerial::VirtualSerial()
  : _buf(nullptr), _cap(0), _pos(0), _count(0), _peer(nullptr) {
  _buf = (uint8_t*)malloc(64);
  _cap = _buf ? 64 : 0;
}

VirtualSerial::~VirtualSerial() { free(_buf); }

void VirtualSerial::connect(VirtualSerial* peer) {
  _peer = peer;
}

int VirtualSerial::available() {
  return _peer ? (int)_peer->_count : 0;
}

int VirtualSerial::read() {
  if (!_peer || _peer->_count == 0) return -1;
  int c = _peer->_buf[_peer->_pos];
  _peer->_pos = (_peer->_pos + 1) % _peer->_cap;
  _peer->_count--;
  return c;
}

size_t VirtualSerial::write(uint8_t c) {
  if (_count >= _cap) {
    size_t newCap = _cap ? _cap * 2 : 64;
    uint8_t* newBuf = (uint8_t*)realloc(_buf, newCap);
    if (!newBuf) return 0;
    if (_pos + _count > _cap) {
      // Unwrap wrapped data
      size_t tail = _cap - _pos;
      memmove(newBuf + tail, newBuf, _pos);
      _pos += tail;
    }
    _buf = newBuf;
    _cap = newCap;
  }
  size_t idx = (_pos + _count) % _cap;
  _buf[idx] = c;
  _count++;
  return 1;
}

size_t VirtualSerial::write(const uint8_t* buf, size_t len) {
  for (size_t i = 0; i < len; i++) write(buf[i]);
  return len;
}

void VirtualSerial::flush() {}

void VirtualSerial::end() {
  free(_buf);
  _buf = nullptr;
  _cap = _pos = _count = 0;
  _peer = nullptr;
}
