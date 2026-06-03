#pragma once
// Mock: RTC/Clock.h
#include <Arduino.h>

struct MockDateTime {
  int _hour = 12, _minute = 34, _second = 0;
  int _day = 1, _month = 6, _year = 2026, _dayOfWeek = 1;
  int hour() const { return _hour; }
  int minute() const { return _minute; }
  int second() const { return _second; }
  int day() const { return _day; }
  int month() const { return _month; }
  int year() const { return _year; }
  int dayOfTheWeek() const { return _dayOfWeek; }
};

class ClockClass {
public:
  void begin() {}
  void update() {}
  bool lostPower() { return false; }
  MockDateTime get() const {
    static int tick = 0; tick++;
    MockDateTime dt;
    dt._minute = (tick / 60) % 60;
    dt._hour = (tick / 3600) % 24;
    return dt;
  }
};
extern ClockClass Clock;
