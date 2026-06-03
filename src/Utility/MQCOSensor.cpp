#include "MQCOSensor.h"
#include "../cfg/pins.h"
#include "../cfg/DFConfig.h"

#if USE_MQ7 == 1

#if defined(ESP32)
#include <driver/adc.h>
#include <esp_adc_cal.h>
#endif

#include <math.h>

// Voltage divider: 10k + 10k from sensor to ADC pin
static const float DIVIDER_RATIO = 2.0f;  // V_sensor = V_adc * 2

// ADC reference voltage (ESP32-S3 internal)
static const float ADC_VREF = 3.3f;
static const int   ADC_RES = 4095;

// MQ-7 sensitivity curve for CO (log-log approximation)
// Derived from datasheet: log10(PPM) = A + B * log10(Rs/R0)
// A = -0.73, B = -2.76
static const float CURVE_A = -0.73f;
static const float CURVE_B = -2.76f;

CMQCOSensor::CMQCOSensor() {}

void CMQCOSensor::begin() {
#if defined(ESP32)
  adc1_config_width(ADC_WIDTH_12Bit);
  // GPIO4 = ADC1_CHANNEL_3 on ESP32-S3
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_11db);
#endif
  _ready = true;
}

void CMQCOSensor::read() {
#if defined(ESP32)
  int raw = adc1_get_raw(ADC1_CHANNEL_3);
#else
  int raw = 0;
#endif

  // Exponential moving average filter
  if (_sampleCount < 10) {
    // Bootstrap during first samples
    _raw = (_raw * _sampleCount + raw) / (_sampleCount + 1);
    _sampleCount++;
  } else {
    _raw = _raw * 0.7f + raw * 0.3f;
  }

  // Convert filtered raw value to pin voltage
  _pinV = (float)_raw * ADC_VREF / (float)ADC_RES;

  // Correct for voltage divider to get sensor output voltage
  _sensorV = _pinV * DIVIDER_RATIO;

  // Compute Rs/R0
  // Rs = (Vc - Vrl) * Rl / Vrl
  // where Vc = 5V (heater/sensor circuit voltage)
  //       Vrl = V_sensor (voltage across load)
  //       Rl = 10k (load resistor on module)
  float Vc = 5.0f;
  float Vrl = _sensorV;
  if (Vrl > 0.01f && _R0 > 0.01f) {
    float Rs = (Vc - Vrl) * 10.0f / Vrl;  // Rs in kOhm
    _ratio = Rs / _R0;
  }

  // Convert Rs/R0 to PPM using the sensitivity curve
  if (_ratio > 0.01f) {
    _ppm = powf(10.0f, CURVE_A + CURVE_B * log10f(_ratio));
    if (_ppm > 10000.0f) _ppm = 10000.0f;
    if (_ppm < 0.1f) _ppm = 0.0f;
  } else {
    _ppm = 0.0f;
  }
}

bool CMQCOSensor::getAlarmState() const {
  return digitalRead(MQ7_DOUT) == LOW;  // Active-low on most modules
}

void CMQCOSensor::calibrate() {
  // In clean air, Rs/R0 ≈ 1.0, so R0 ≈ Rs
  float Vc = 5.0f;
  float Vrl = _sensorV;
  if (Vrl > 0.01f) {
    _R0 = (Vc - Vrl) * 10.0f / Vrl;
    // Clamp reasonable range
    if (_R0 < 0.1f) _R0 = 0.1f;
    if (_R0 > 100.0f) _R0 = 100.0f;
  }
}

#else
// MQ-7 disabled - all methods are no-ops
CMQCOSensor::CMQCOSensor() {}
void CMQCOSensor::begin() {}
void CMQCOSensor::read() {}
bool CMQCOSensor::getAlarmState() const { return false; }
void CMQCOSensor::calibrate() {}
#endif
