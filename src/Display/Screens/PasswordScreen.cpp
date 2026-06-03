#include "PasswordScreen.h"
#include <cstring>
#include "../../cfg/DFConfig.h"

static const int PASSWORD[4] = {1, 6, 8, 8};

PasswordScreen::PasswordScreen(void (*onSuccess)(), DieselScreen* next)
  : DieselScreen("Password"), _onSuccess(onSuccess), _nextScreen(next) {}

void PasswordScreen::onLoad() {
  createHeader(_screen);
  hideBackButton();
  _pos = 0;

  createLabel(_screen, "Enter PIN", LV_ALIGN_TOP_MID, 0, 28);

  // Digit dots
  for (int i = 0; i < 4; i++) {
    _dots[i] = lv_label_create(_screen);
    lv_obj_set_pos(_dots[i], 100 + i * 32, 60);
    lv_obj_set_style_text_color(_dots[i], lv_color_white(), 0);
    lv_obj_set_style_text_font(_dots[i], &lv_font_montserrat_24, 0);
    lv_label_set_text(_dots[i], "_");
  }

  // Numeric keypad
  static const char* keys[] = {"1","2","3","4","5","6","7","8","9","*","0","#",""};
  _btnGrid = lv_btnmatrix_create(_screen);
  lv_btnmatrix_set_map(_btnGrid, keys);
  lv_obj_set_pos(_btnGrid, 40, 96);
  lv_obj_set_size(_btnGrid, TFT_WIDTH - 80, 140);
  lv_obj_add_event_cb(_btnGrid, onDigitClick, LV_EVENT_VALUE_CHANGED, this);
}

void PasswordScreen::onDigitClick(lv_event_t* e) {
  auto* self = static_cast<PasswordScreen*>(lv_event_get_user_data(e));
  auto* btnm = static_cast<lv_obj_t*>(lv_event_get_target(e));
  uint32_t id = lv_btnmatrix_get_selected_btn(btnm);
  int digit = "123456789*0#"[id] - '0';

  if (digit < 0 || digit > 9) {
    if (id == 9) { // * = clear
      self->_pos = 0;
      memset(self->_digits, 0, sizeof(self->_digits));
      for (int i = 0; i < 4; i++) lv_label_set_text(self->_dots[i], "_");
    }
    return;
  }

  if (self->_pos < 4) {
    self->_digits[self->_pos] = digit;
    char buf[2] = {(char)('0' + digit), 0};
    lv_label_set_text(self->_dots[self->_pos], buf);
    self->_pos++;
  }

  if (self->_pos == 4) self->checkPassword();
}

void PasswordScreen::checkPassword() {
  bool ok = true;
  for (int i = 0; i < 4; i++) {
    if (_digits[i] != PASSWORD[i]) { ok = false; break; }
  }
  if (ok && _nextScreen) {
    _nextScreen->onLoad();
    lv_scr_load(_nextScreen->getScreen());
  } else {
    _pos = 0;
    memset(_digits, 0, sizeof(_digits));
    for (int i = 0; i < 4; i++) lv_label_set_text(_dots[i], "_");
  }
}
