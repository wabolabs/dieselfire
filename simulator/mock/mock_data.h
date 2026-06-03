#ifndef MOCK_HEATER_INFO_H
#define MOCK_HEATER_INFO_H

#include <cstdint>
#include <cstring>

// Simulated heater frame (subset of CProtocol's Heater struct)
struct HeaterFrame {
  uint8_t Byte0 = 0x76;
  uint8_t Len = 22;
  uint8_t Command = 0;
  int8_t ActualTemperature = 20;
  int8_t DesiredDemand = 20;
  uint8_t MinPumpFreq = 14;
  uint8_t MaxPumpFreq = 45;
  uint8_t MinFanRPM_MSB = 0;
  uint8_t MinFanRPM_LSB = 150;
  uint8_t MaxFanRPM_MSB = 0;
  uint8_t MaxFanRPM_LSB = 200;
  uint8_t OperatingVoltage = 120;
  uint8_t FanSensor = 1;
  uint8_t OperatingMode = 0x32;
  int8_t MinTemperature = 8;
  int8_t MaxTemperature = 35;
  uint8_t GlowDrive = 5;
  uint8_t Prime = 0;
  uint8_t Unknown1_MSB = 1;
  uint8_t Unknown1_LSB = 0x2c;
  int8_t Altitude_MSB = 0;
  uint8_t Altitude_LSB = 0;
  uint8_t CRC_MSB = 0;
  uint8_t CRC_LSB = 0;
};

// Simulated heater run state
struct SimState {
  int runState = 0;          // 0=off, 1=starting, 2=glow, 3=ignited, 4=running, 5=shutdown
  int errState = 0;
  float ambientTemp = 10.0f;
  float heatExchangerTemp = 15.0f;
  float pumpHz = 0.0f;
  int fanRPM = 0;
  float glowPower = 0.0f;
  float batteryVolts = 12.6f;
  uint16_t fanVoltage = 0;
  float glowCurrent = 0.0f;
  float glowVoltage = 0.0f;
  float fuelUsed_liters = 0.0f;
  int demand = 20;           // desired temp in tenths
  bool thermostat = true;
  bool fixedHz = false;
};

extern SimState g_state;
void updateSimulation(int ms);
#endif
