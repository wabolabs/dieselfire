#include "DieselScreen.h"
#include "../../cfg/DFConfig.h"
#include "../../cfg/pins.h"
#include "../../RTC/Clock.h"
#include "../../Utility/DebugPort.h"

lv_group_t* DieselScreen::_navGroup = nullptr;
DieselScreen* DieselScreen::_currentScreen = nullptr;

// ── Theme colors ────────────────────────────────────────────
static const lv_color_t C_BG      = lv_color_make(0x12, 0x12, 0x12);
static const lv_color_t C_HEADER  = lv_color_make(0x1A, 0x1A, 0x1A);
static const lv_color_t C_AMBER   = lv_color_make(0xFF, 0x8C, 0x00);
static const lv_color_t C_WHITE   = lv_color_make(0xFF, 0xFF, 0xFF);
static const lv_color_t C_GREY    = lv_color_make(0x88, 0x88, 0x88);
static const lv_color_t C_GREEN   = lv_color_make(0x00, 0xCC, 0x00);
static const lv_color_t C_RED     = lv_color_make(0xCC, 0x00, 0x00);
static const lv_color_t C_BLUE    = lv_color_make(0x00, 0xAA, 0xFF);

DieselScreen::DieselScreen(const char* name) : _name(name) {
  _screen = lv_obj_create(lv_screen_active());
  lv_obj_set_style_bg_color(_screen, C_BG, 0);
  lv_obj_set_scrollbar_mode(_screen, LV_SCROLLBAR_MODE_OFF);
}

DieselScreen::~DieselScreen() {
  if (_timer) lv_timer_del(_timer);
}

void DieselScreen::initTheme() {
  lv_theme_t* th = lv_theme_default_init(
    lv_display_get_default(),
    lv_palette_main(LV_PALETTE_ORANGE),
    lv_palette_main(LV_PALETTE_GREY),
    false,
    LV_FONT_DEFAULT
  );
  lv_display_set_theme(lv_display_get_default(), th);
}

void DieselScreen::setNavGroup(lv_group_t* group) {
  _navGroup = group;
}

void DieselScreen::onLoad() {}
void DieselScreen::onUnload() {}

void DieselScreen::onTimer() {
  updateHeaderClock();
}

void DieselScreen::pushScreen(DieselScreen* screen) {
  _currentScreen = screen;
  lv_scr_load(screen->getScreen());
  screen->onLoad();
}

void DieselScreen::popScreen() {
  // TODO: nav stack
  showHome();
}

void DieselScreen::showHome() {
  // First screen in nav stack
}

void DieselScreen::hideBackButton() {
  if (_headerBack) lv_obj_add_flag(_headerBack, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t* DieselScreen::createHeader(lv_obj_t* parent) {
  _header = lv_obj_create(parent);
  lv_obj_set_size(_header, TFT_WIDTH, 22);
  lv_obj_set_pos(_header, 0, 0);
  lv_obj_set_style_bg_color(_header, C_HEADER, 0);
  lv_obj_set_style_border_width(_header, 0, 0);
  lv_obj_set_style_pad_all(_header, 0, 0);
  lv_obj_set_scrollbar_mode(_header, LV_SCROLLBAR_MODE_OFF);

  _headerBack = lv_label_create(_header);
  lv_label_set_text(_headerBack, LV_SYMBOL_LEFT);
  lv_obj_set_style_text_color(_headerBack, C_AMBER, 0);
  lv_obj_set_pos(_headerBack, 4, 2);

  _headerClock = lv_label_create(_header);
  lv_label_set_text(_headerClock, "--:--");
  lv_obj_set_style_text_color(_headerClock, C_WHITE, 0);
  lv_obj_center(_headerClock);

  _headerHeater = lv_label_create(_header);
  lv_label_set_text(_headerHeater, LV_SYMBOL_POWER);
  lv_obj_set_style_text_color(_headerHeater, C_GREY, 0);
  lv_obj_set_pos(_headerHeater, TFT_WIDTH - 10, 2);
  lv_obj_set_style_text_align(_headerHeater, LV_TEXT_ALIGN_RIGHT, 0);

  _headerWifi = lv_label_create(_header);
  lv_label_set_text(_headerWifi, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_color(_headerWifi, C_GREY, 0);
  lv_obj_set_pos(_headerWifi, TFT_WIDTH - 28, 2);

  _headerBt = lv_label_create(_header);
  lv_label_set_text(_headerBt, LV_SYMBOL_BLUETOOTH);
  lv_obj_set_style_text_color(_headerBt, C_GREY, 0);
  lv_obj_set_pos(_headerBt, TFT_WIDTH - 46, 2);

  _headerBattery = lv_label_create(_header);
  lv_label_set_text(_headerBattery, LV_SYMBOL_BATTERY_FULL);
  lv_obj_set_style_text_color(_headerBattery, C_GREEN, 0);
  lv_obj_set_pos(_headerBattery, TFT_WIDTH - 64, 2);

  return _header;
}

void DieselScreen::updateHeaderClock() {
  if (!_headerClock) return;
  // Clock.update() called from main loop; just read latest
  char buf[6];
  int h = Clock.get().hour();
  int m = Clock.get().minute();
  snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
  lv_label_set_text(_headerClock, buf);
}

void DieselScreen::updateHeaderBtIcon(bool connected) {
  if (!_headerBt) return;
  lv_obj_set_style_text_color(_headerBt, connected ? C_BLUE : C_GREY, 0);
}

void DieselScreen::updateHeaderWifiIcon(int rssi) {
  if (!_headerWifi) return;
  lv_obj_set_style_text_color(_headerWifi, rssi > -80 ? C_GREEN : C_GREY, 0);
}

void DieselScreen::updateHeaderBattery(float volts) {
  if (!_headerBattery) return;
  const char* sym;
  if (volts > 13.0f) sym = LV_SYMBOL_BATTERY_FULL;
  else if (volts > 12.5f) sym = LV_SYMBOL_BATTERY_3;
  else if (volts > 12.0f) sym = LV_SYMBOL_BATTERY_2;
  else if (volts > 11.5f) sym = LV_SYMBOL_BATTERY_1;
  else sym = LV_SYMBOL_BATTERY_EMPTY;
  lv_label_set_text(_headerBattery, sym);
  lv_obj_set_style_text_color(_headerBattery, volts > 12.0f ? C_GREEN : C_RED, 0);
}

void DieselScreen::updateHeaderHeaterState(int state) {
  if (!_headerHeater) return;
  if (state >= 1 && state <= 5) {
    lv_obj_set_style_text_color(_headerHeater, C_RED, 0);
  } else {
    lv_obj_set_style_text_color(_headerHeater, C_GREY, 0);
  }
}

lv_obj_t* DieselScreen::createLabel(lv_obj_t* parent, const char* text, lv_align_t align, lv_coord_t x, lv_coord_t y) {
  lv_obj_t* lbl = lv_label_create(parent);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_color(lbl, C_WHITE, 0);
  lv_obj_align(lbl, align, x, y);
  return lbl;
}

lv_obj_t* DieselScreen::createBigLabel(lv_obj_t* parent, const char* text, lv_align_t align, lv_coord_t x, lv_coord_t y) {
  lv_obj_t* lbl = lv_label_create(parent);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_color(lbl, C_AMBER, 0);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_36, 0);
  lv_obj_align(lbl, align, x, y);
  return lbl;
}

lv_obj_t* DieselScreen::createValueLabel(lv_obj_t* parent, const char* text, lv_align_t align, lv_coord_t x, lv_coord_t y) {
  lv_obj_t* lbl = lv_label_create(parent);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_color(lbl, C_WHITE, 0);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, 0);
  lv_obj_align(lbl, align, x, y);
  return lbl;
}

lv_obj_t* DieselScreen::createUnitLabel(lv_obj_t* parent, const char* text, lv_align_t align, lv_coord_t x, lv_coord_t y) {
  lv_obj_t* lbl = lv_label_create(parent);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_color(lbl, C_GREY, 0);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_10, 0);
  lv_obj_align(lbl, align, x, y);
  return lbl;
}

lv_obj_t* DieselScreen::createLed(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_color_t color) {
  lv_obj_t* led = lv_led_create(parent);
  lv_obj_set_pos(led, x, y);
  lv_obj_set_size(led, 8, 8);
  lv_led_set_color(led, color);
  lv_led_on(led);
  return led;
}

lv_obj_t* DieselScreen::createBar(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, int16_t range_max) {
  lv_obj_t* bar = lv_bar_create(parent);
  lv_obj_set_pos(bar, x, y);
  lv_obj_set_size(bar, w, h);
  lv_bar_set_range(bar, 0, range_max);
  lv_obj_set_style_bg_color(bar, lv_color_make(0x33, 0x33, 0x33), 0);
  lv_obj_set_style_bg_color(bar, C_AMBER, LV_PART_INDICATOR);
  return bar;
}

lv_obj_t* DieselScreen::createNavButton(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y) {
  lv_obj_t* btn = lv_button_create(parent);
  lv_obj_set_pos(btn, x, y);
  lv_obj_set_style_bg_color(btn, lv_color_make(0x33, 0x33, 0x33), 0);
  lv_obj_set_style_bg_color(btn, C_AMBER, LV_STATE_PRESSED);
  lv_obj_t* lbl = lv_label_create(btn);
  lv_label_set_text(lbl, text);
  lv_obj_center(lbl);
  return btn;
}
