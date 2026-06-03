#pragma once
class CExpMean {
public:
  void reset(float) {}
  void update(float) {}
  float getValue() const { return 20.0f; }
  void setRounding(float) {}
};

struct sFilteredData {
  CExpMean AmbientTemp;
  CExpMean ipVolts;
  CExpMean GlowAmps;
  CExpMean GlowVolts;
  CExpMean Fan;
  CExpMean FastipVolts;
  CExpMean FastGlowAmps;
};

extern sFilteredData FilteredSamples;
