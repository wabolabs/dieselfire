#pragma once

#include "DieselScreen.h"

class SettingsMenuScreen : public DieselScreen {
public:
  SettingsMenuScreen();
  void onLoad() override;

private:
  static void onEntryClick(lv_event_t* e);
  void openScreen(int index);
};
