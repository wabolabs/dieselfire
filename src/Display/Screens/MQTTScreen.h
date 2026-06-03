#pragma once

#include "DieselScreen.h"

class MQTTScreen : public DieselScreen {
public:
  MQTTScreen();
  void onLoad() override;
};
