#ifndef __MQCO_SENSOR_H__
#define __MQCO_SENSOR_H__

#include <stdint.h>

class CMQCOSensor {
public:
  CMQCOSensor();

  // Initialize ADC. Call once from setup().
  void begin();

  // Read the current analog value and update internal filter.
  // Call periodically (every ~100ms).
  void read();

  // Get the latest filtered CO concentration in PPM.
  float getPPM() const { return _ppm; }

  // Get the latest filtered raw ADC value (0-4095).
  int getRawADC() const { return _raw; }

  // Get the latest filtered ADC voltage at the ESP32 pin (after divider).
  float getPinVoltage() const { return _pinV; }

  // Get the estimated sensor voltage (before divider).
  float getSensorVoltage() const { return _sensorV; }

  // Get the digital threshold state (DOUT pin).
  bool getAlarmState() const;

  // Get the ratio Rs/R0 (clean air reference).
  float getRatio() const { return _ratio; }

  // Calibration: store current reading as clean-air reference (R0).
  // Call in clean air after pre-heat.
  void calibrate();

  // Get/set stored calibration values.
  float getR0() const { return _R0; }
  void setR0(float r0) { _R0 = r0; }

  // Check if the sensor has been pre-heated enough (48h initial, shorter subsequently).
  bool isReady() const { return _ready; }

private:
  float _ppm = 0.0f;
  int   _raw = 0;
  float _pinV = 0.0f;
  float _sensorV = 0.0f;
  float _ratio = 1.0f;
  float _R0 = 10.0f;         // Clean air reference resistance
  bool  _ready = false;
  uint8_t _sampleCount = 0;
};

#endif
