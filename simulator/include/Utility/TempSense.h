#pragma once
// Mock: Utility/TempSense.h
#include <cstdint>

class CBME280Sensor {
public:
  int getCount() { return 1; }
  float getTemperature() { return 22.5f; }
};

class CDS18B20SensorSet {
public:
  int getNumSensors() { return 1; }
  int getTemperatureIdx(int) { return 0; }
  float getTemperature(int) { return 18.3f; }
  bool getRomCodeIdx(int, void*) { return false; }
};

class CTempSense {
public:
  void begin(int, int) {}
  void startConvert() {}
  void readSensors() {}
  bool getTemperature(int, float& t, bool = true) { t = 20.5f; return true; }
  bool getHumidity(float& h) { h = 55.0f; return true; }
  bool getAltitude(float& a) { a = 120.0f; return true; }
  CBME280Sensor& getBME280() { static CBME280Sensor s; return s; }
  CDS18B20SensorSet& getDS18B20() { static CDS18B20SensorSet s; return s; }
};
extern CTempSense TempSensor;
