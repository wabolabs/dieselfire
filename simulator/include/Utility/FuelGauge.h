#pragma once
// Mock: Utility/FuelGauge.h
class CFuelGauge {
public:
  void init(float) {}
  float Used_mL() const { return 0.0f; }
  void reset() {}
  void Integrate(float) {}
};
extern CFuelGauge FuelGauge;
