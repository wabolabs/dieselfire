#include "InheritScreen.h"

InheritScreen::InheritScreen() : DieselScreen("Inherit") {}

void InheritScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Inherit Settings", LV_ALIGN_TOP_LEFT, 48, 3);

  createLabel(_screen,
    "Connect OEM controller\nto copy its tuning\nsettings.",
    LV_ALIGN_CENTER, 0, -20);
}
