#include "RebootScreen.h"
#include <cstdio>

RebootScreen::RebootScreen() : DieselScreen("Reboot") {}

void RebootScreen::onLoad() {
  createHeader(_screen);
  hideBackButton();
  _label = createBigLabel(_screen, "Rebooting...", LV_ALIGN_CENTER, 0, -20);
  _timer = lv_timer_create([](lv_timer_t* t) {
    static_cast<RebootScreen*>(lv_timer_get_user_data(t))->onTimer();
  }, 500, this);
}

void RebootScreen::onTimer() {
  char buf[16];
  snprintf(buf, sizeof(buf), "Reboot in %d", _countdown);
  lv_label_set_text(_label, buf);
  _countdown--;
}
