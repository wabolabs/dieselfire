// Force-included before all other headers in simulator build
// This provides stubs for ESP32/Arduino types so the real header tree compiles

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <cstdio>
#include <cstring>

// ===== Core types =====
typedef uint8_t byte;
typedef bool boolean;

// ===== GPIO =====
enum { GPIO_NUM_0, GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_3, GPIO_NUM_4,
       GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7, GPIO_NUM_8, GPIO_NUM_9,
       GPIO_NUM_10, GPIO_NUM_11, GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14,
       GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_19,
       GPIO_NUM_20, GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_23, GPIO_NUM_24,
       GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_28, GPIO_NUM_29,
       GPIO_NUM_30, GPIO_NUM_31, GPIO_NUM_32, GPIO_NUM_33, GPIO_NUM_34,
       GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37, GPIO_NUM_38, GPIO_NUM_39,
       GPIO_NUM_40, GPIO_NUM_41, GPIO_NUM_42, GPIO_NUM_43, GPIO_NUM_44,
       GPIO_NUM_45, GPIO_NUM_46, GPIO_NUM_47, GPIO_NUM_48 };
typedef int gpio_num_t;

// ===== Arduino I/O =====
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define LOW 0
#define HIGH 1
#define CHANGE 1
static inline void pinMode(int,int) {}
static inline void digitalWrite(int,int) {}
static inline int digitalRead(int) { return 0; }
static inline unsigned long millis() { return 0; }
static inline void delay(unsigned long) {}
static inline void* ps_malloc(size_t s) { return malloc(s); }

// ===== ADC =====
typedef int adc1_channel_t;
typedef int adc2_channel_t;
typedef int adc_channel_t;
typedef int adc_atten_t;
typedef int adc_unit_t;
#define ADC1_CHANNEL_2 2
#define ADC1_CHANNEL_5 5
#define ADC1_CHANNEL_0 0
#define ADC_ATTEN_11db 3
#define ADC_UNIT_1 1
#define ADC_WIDTH_12Bit 3

// ===== FreeRTOS =====
typedef void* TaskHandle_t;

// ===== RMT =====
typedef int rmt_channel_t;
#define RMT_CHANNEL_4 (rmt_channel_t)4

// ===== Serial =====
#define SERIAL_8N1 0x01

class HardwareSerial {
public:
  void begin(unsigned long, uint32_t conf=0, int8_t rx=-1, int8_t tx=-1, bool inv=false) {}
  void end() {}
  int available() { return 0; }
  int read() { return -1; }
  size_t write(uint8_t) { return 1; }
  size_t write(const uint8_t*, size_t s) { return s; }
  void print(const char*) {}
  void println(const char*) {}
  void printf(const char*, ...) {}
  void flush() {}
  operator bool() { return false; }
};
extern HardwareSerial Serial;
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;

// ===== Print / Stream =====
class Print {
public:
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t*, size_t s) { return s; }
  void print(const char*) {}
  void println(const char*) {}
  void println(int) {}
  void println(float) {}
  void printf(const char*, ...) {}
  virtual void flush() {}
};

class Stream : public Print {
public:
  int available() { return 0; }
  int read() { return -1; }
  size_t readBytes(char*, size_t) { return 0; }
};

// ===== String =====
class String {
  char _buf[128]{};
public:
  String() {}
  String(const char* s) { strncpy(_buf, s, 127); }
  operator const char*() const { return _buf; }
  const char* c_str() const { return _buf; }
  int length() const { return strlen(_buf); }
  bool operator==(const String& o) const { return strcmp(_buf, o._buf) == 0; }
};

// ===== WiFi =====
typedef int wl_status_t;
#define WL_CONNECTED 3
#define WL_IDLE_STATUS 0

class WiFiClient {
public:
  int connect(const char*, int) { return 0; }
  size_t write(const uint8_t*, size_t s) { return s; }
  int available() { return 0; }
  int read() { return -1; }
  void stop() {}
  uint8_t connected() { return 0; }
  operator bool() { return false; }
};

class WiFiServer {
public:
  WiFiServer(int) {}
  void begin() {}
  bool available() { return false; }
};

typedef int arduino_event_id_t;
typedef int arduino_event_info_t;
struct arduino_event_t { arduino_event_id_t event_id; arduino_event_info_t event_info; };

class WiFiClass {
public:
  void onEvent(void (*)(void*, arduino_event_t*)) {}
  void onEvent(void (*)(void*, arduino_event_t*), int, void*) {}
  int begin(const char*, const char*) { return WL_CONNECTED; }
  wl_status_t status() { return WL_CONNECTED; }
  int RSSI() { return -50; }
};
extern WiFiClass WiFi;

class IPAddress {
public:
  IPAddress() {}
  IPAddress(uint32_t) {}
  operator uint32_t() { return 0; }
};

// ===== Preferences =====
class Preferences {
public:
  bool begin(const char*, bool) { return true; }
  void end() {}
  uint8_t getUChar(const char*, uint8_t d) { return d; }
  int32_t getInt(const char*, int32_t d) { return d; }
  uint32_t getUInt(const char*, uint32_t d) { return d; }
  float getFloat(const char*, float d) { return d; }
  bool getBool(const char*, bool d) { return d; }
  bool putUChar(const char*, uint8_t) { return true; }
  bool putInt(const char*, int32_t) { return true; }
  bool putBytes(const char*, const void*, size_t) { return true; }
  size_t getBytesLength(const char*) { return 0; }
  size_t getBytes(const char*, void*, size_t) { return 0; }
};

// ===== ESP-IDF =====
typedef uint32_t esp_err_t;
#define ESP_OK 0
#define GPIO_NUM_22 22

// ===== Wire =====
class TwoWire {
public:
  void begin(int, int) {}
  void beginTransmission(int) {}
  int endTransmission() { return 0; }
  size_t write(uint8_t) { return 1; }
  void requestFrom(int, int) {}
  int available() { return 0; }
  int read() { return 0; }
};
extern TwoWire Wire;
extern TwoWire Wire1;
