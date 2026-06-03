#pragma once
// Mock: RTC/Timers.h
#include <cstdint>

struct sHourMin {
  int8_t hour = 0;
  int8_t min = 0;
};

struct sTimer {
  sHourMin start;
  sHourMin stop;
  uint8_t enabled = 0;
  uint8_t repeat = 0;
  uint8_t temperature = 22;
  uint8_t timerID = 0;
  void init(int idx) {
    start.hour = 0; start.min = 0;
    stop.hour = 0; stop.min = 0;
    enabled = 0; repeat = 0;
    temperature = 22; timerID = idx;
  }
};
