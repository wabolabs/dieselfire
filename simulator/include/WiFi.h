#pragma once
// Mock: WiFi.h
#include <stdint.h>
#include <cstddef>
#include "Arduino.h"

typedef int wl_status_t;
#define WL_CONNECTED 3
#define WL_IDLE_STATUS 0
typedef int arduino_event_t;

class WiFiClass {
public:
  int begin(const char*, const char*) { return WL_CONNECTED; }
  void disconnect() {}
  wl_status_t status() { return WL_CONNECTED; }
  int RSSI() { return -50; }
  int hostByName(const char*) { return 1; }
  void mode(int) {}
  void softAP(const char*) {}
  void softAP(const char*, const char*) {}
  void printDiag(HardwareSerial&) {}
  void setSleep(bool) {}
  void setAutoReconnect(bool) {}
  void onEvent(void(*)(void*, arduino_event_t*)) {}
  void onEvent(void(*)(void*, arduino_event_t*), int, void*) {}
};
extern WiFiClass WiFi;

typedef struct {
  uint8_t addr[6];
} macAddress;

typedef struct {
  uint16_t count;
  uint8_t* bssid;
  int32_t rssi;
  uint8_t* ssid;
} WiFiResult;

// Event types (stubs)
typedef int arduino_event_id_t;
typedef int arduino_event_info_t;
typedef struct { arduino_event_id_t event_id; arduino_event_info_t event_info; } arduino_event_t;

class WiFiServer {
public:
  WiFiServer(int) {}
  void begin() {}
  void close() {}
  bool available() { return false; }
  int setTimeout(int) { return 0; }
};

class WiFiClient {
public:
  WiFiClient() {}
  WiFiClient(const WiFiClient&) {}
  int connect(const char*, int) { return 0; }
  int connect(uint32_t, int) { return 0; }
  size_t write(const uint8_t*, size_t s) { return s; }
  int available() { return 0; }
  int read() { return -1; }
  int read(uint8_t*, size_t) { return 0; }
  void flush() {}
  void stop() {}
  uint8_t connected() { return 0; }
  operator bool() { return false; }
  WiFiClient& operator=(const WiFiClient&) { return *this; }
};

class IPAddress {
public:
  IPAddress() {}
  IPAddress(uint32_t) {}
  IPAddress(uint8_t, uint8_t, uint8_t, uint8_t) {}
  operator uint32_t() { return 0; }
};

class WiFiUDP {
public:
  uint8_t begin(int) { return 1; }
  void stop() {}
  int beginPacket(const char*, int) { return 0; }
  int beginPacket(IPAddress, int) { return 0; }
  int endPacket() { return 0; }
  size_t write(const uint8_t*, size_t s) { return s; }
  int parsePacket() { return 0; }
  int read(uint8_t*, size_t) { return 0; }
  int read() { return -1; }
  int available() { return 0; }
};
