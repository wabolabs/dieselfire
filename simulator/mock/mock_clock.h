#ifndef MOCK_CLOCK_H
#define MOCK_CLOCK_H

#include <cstdint>

struct MockDateTime {
  int _hour = 12;
  int _minute = 34;
  int _second = 0;
  int _day = 1;
  int _month = 6;
  int _year = 2026;
  int _dayOfWeek = 1;

  int hour() const { return _hour; }
  int minute() const { return _minute; }
  int second() const { return _second; }
  int day() const { return _day; }
  int month() const { return _month; }
  int year() const { return _year; }
  int dayOfTheWeek() const { return _dayOfWeek; }
};

class MockClock {
public:
  void begin() {}
  void update() {}
  bool lostPower() { return false; }
  MockDateTime get() const { return _now; }

private:
  MockDateTime _now;
};

extern MockClock Clock;
#endif
