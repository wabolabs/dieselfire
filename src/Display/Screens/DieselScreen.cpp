#include "DieselScreen.h"
#include "../../cfg/DFConfig.h"
#include "../../cfg/pins.h"
#include "../../RTC/Clock.h"
#include "../../Utility/DebugPort.h"

lv_group_t* DieselScreen::_navGroup = nullptr;

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
  _screen = lv_obj_create(NULL);  // NULL parent = standalone screen object
  lv_obj_set_size(_screen, TFT_WIDTH, TFT_HEIGHT);
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
  lv_scr_load(screen->getScreen());
  screen->onLoad();
}

void DieselScreen::popScreen() {
  // TODO: nav stack
  showHome();
}

void DieselScreen::onBack() {
  if (_backCallback) _backCallback();
}

void DieselScreen::showHome() {
  // First screen in nav stack
}

void DieselScreen::hideBackButton() {
  if (_headerBack) lv_obj_add_flag(_headerBack, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t* DieselScreen::createContentArea() {
  lv_obj_t* cont = lv_obj_create(_screen);
  lv_obj_set_size(cont, TFT_WIDTH, TFT_HEIGHT - 22);
  lv_obj_set_pos(cont, 0, 22);
  lv_obj_set_style_bg_color(cont, C_BG, 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_AUTO);
  return cont;
}

lv_obj_t* DieselScreen::createHeader(lv_obj_t* parent) {
  _header = lv_obj_create(parent);
  lv_obj_set_size(_header, TFT_WIDTH, 22);
  lv_obj_set_pos(_header, 0, 0);
  lv_obj_set_style_bg_color(_header, C_HEADER, 0);
  lv_obj_set_style_border_width(_header, 0, 0);
  lv_obj_set_style_pad_all(_header, 0, 0);
  lv_obj_set_scrollbar_mode(_header, LV_SCROLLBAR_MODE_OFF);

  // Back button (left)
  _headerBack = lv_label_create(_header);
  lv_label_set_text(_headerBack, LV_SYMBOL_LEFT);
  lv_obj_set_style_text_color(_headerBack, C_AMBER, 0);
  lv_obj_set_pos(_headerBack, 4, 0);
  lv_obj_set_size(_headerBack, 22, 22);
  lv_obj_add_flag(_headerBack, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(_headerBack, [](lv_event_t* e) {
    static_cast<DieselScreen*>(lv_event_get_user_data(e))->onBack();
  }, LV_EVENT_CLICKED, this);

  // Clock (center)
  _headerClock = lv_label_create(_header);
  lv_label_set_text(_headerClock, "--:--");
  lv_obj_set_style_text_color(_headerClock, C_WHITE, 0);
  lv_obj_center(_headerClock);

  // Settings gear (right)
  _headerSettings = lv_label_create(_header);
  lv_label_set_text(_headerSettings, LV_SYMBOL_SETTINGS);
  lv_obj_set_style_text_color(_headerSettings, C_GREY, 0);
  lv_obj_set_pos(_headerSettings, TFT_WIDTH - 24, 0);
  lv_obj_set_size(_headerSettings, 22, 22);
  lv_obj_add_flag(_headerSettings, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(_headerSettings, [](lv_event_t* e) {
    static_cast<DieselScreen*>(lv_event_get_user_data(e))->onSettings();
  }, LV_EVENT_CLICKED, this);

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

void DieselScreen::updateHeaderBtIcon(bool) {}
void DieselScreen::updateHeaderWifiIcon(int) {}
void DieselScreen::updateHeaderBattery(float) {}
void DieselScreen::updateHeaderHeaterState(int) {}

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
