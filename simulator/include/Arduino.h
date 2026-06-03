// Stub for Arduino.h on PC simulator
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>
#include <functional>

// Arduino types
typedef uint8_t byte;
typedef bool boolean;

// GPIO
typedef enum {
  GPIO_NUM_0 = 0,  GPIO_NUM_1 = 1,  GPIO_NUM_2 = 2,
  GPIO_NUM_3 = 3,  GPIO_NUM_4 = 4,  GPIO_NUM_5 = 5,
  GPIO_NUM_6 = 6,  GPIO_NUM_7 = 7,  GPIO_NUM_8 = 8,
  GPIO_NUM_9 = 9,  GPIO_NUM_10 = 10, GPIO_NUM_11 = 11,
  GPIO_NUM_12 = 12, GPIO_NUM_13 = 13, GPIO_NUM_14 = 14,
  GPIO_NUM_15 = 15, GPIO_NUM_16 = 16, GPIO_NUM_17 = 17,
  GPIO_NUM_18 = 18, GPIO_NUM_19 = 19, GPIO_NUM_20 = 20,
  GPIO_NUM_21 = 21, GPIO_NUM_22 = 22, GPIO_NUM_23 = 23,
  GPIO_NUM_24 = 24, GPIO_NUM_25 = 25, GPIO_NUM_26 = 26,
  GPIO_NUM_27 = 27, GPIO_NUM_28 = 28, GPIO_NUM_29 = 29,
  GPIO_NUM_30 = 30, GPIO_NUM_31 = 31, GPIO_NUM_32 = 32,
  GPIO_NUM_33 = 33, GPIO_NUM_34 = 34, GPIO_NUM_35 = 35,
  GPIO_NUM_36 = 36, GPIO_NUM_37 = 37, GPIO_NUM_38 = 38,
  GPIO_NUM_39 = 39, GPIO_NUM_40 = 40, GPIO_NUM_41 = 41,
  GPIO_NUM_42 = 42, GPIO_NUM_43 = 43, GPIO_NUM_44 = 44,
  GPIO_NUM_45 = 45, GPIO_NUM_46 = 46, GPIO_NUM_47 = 47,
  GPIO_NUM_48 = 48
} gpio_num_t;

// ADC
typedef int adc1_channel_t;
typedef int adc2_channel_t;
#define ADC1_CHANNEL_2 2
#define ADC1_CHANNEL_5 5
#define ADC1_CHANNEL_0 0
#define ADC_ATTEN_11db 3
#define ADC_UNIT_1 1
#define ADC_WIDTH_12Bit 3

// FreeRTOS types (full adapter in FreeRTOS.h)
#include <FreeRTOS.h>

// Pin modes
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define LOW 0
#define HIGH 1
#define CHANGE 1

// Misc
#define LED_BUILTIN 2
typedef uint32_t esp_err_t;
#define ESP_OK 0

// PROGMEM (flash storage) is a no-op on Linux simulator
#define PROGMEM
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#define pgm_read_dword(addr) (*(const uint32_t*)(addr))
#define pgm_read_float(addr) (*(const float*)(addr))
#define strlen_P strlen
#define strcmp_P strcmp
#define strncpy_P strncpy
#define sprintf_P sprintf

// Functions
static inline void pinMode(int, int) {}
static inline void digitalWrite(int, int) {}
static inline int digitalRead(int) { return 0; }
static inline unsigned long millis() {
  static auto epoch = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - epoch).count();
}
static inline unsigned long micros() {
  static auto epoch = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(now - epoch).count();
}
static inline void delay(unsigned long ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
static inline void delayMicroseconds(unsigned long us) {
  std::this_thread::sleep_for(std::chrono::microseconds(us));
}
static inline int analogRead(int) { return 0; }
static inline void analogWrite(int, int) {}
static inline void randomSeed(unsigned long) {}
static inline long random(long max) { return 0; }
static inline long random(long min, long max) { return min; }
static inline void* ps_malloc(size_t s) { return malloc(s); }

// ESP-IDF watchdogs (stub)
static inline esp_err_t esp_task_wdt_add(void*) { return ESP_OK; }
static inline esp_err_t esp_task_wdt_reset() { return ESP_OK; }

// RMT
typedef int rmt_channel_t;
#define RMT_CHANNEL_4 (rmt_channel_t)4

// Serial (stub)
class HardwareSerial {
public:
  void begin(int) {}
  void end() {}
  int available() { return 0; }
  int read() { return -1; }
  size_t write(uint8_t) { return 1; }
  size_t write(const uint8_t*, size_t s) { return s; }
  void print(const char*) {}
  void println(const char*) {}
  void printf(const char*, ...) {}
  void operator&() {}
};
extern HardwareSerial Serial;
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;

// String
class String {
public:
  String() {}
  String(const char*) {}
  operator const char*() const { return _buf; }
  int length() const { return strlen(_buf); }
  int indexOf(char) const { return -1; }
  String substring(int) const { return String(); }
  String substring(int, int) const { return String(); }
  bool operator==(const String&) const { return true; }
  const char* c_str() const { return _buf; }
  char _buf[64] = {};
};

// Wire (I2C)
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
