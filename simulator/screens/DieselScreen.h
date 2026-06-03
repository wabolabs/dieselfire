#pragma once

#include <lvgl.h>
#include <misc/lv_timer.h>

class DieselScreen {
public:
  DieselScreen(const char* name);
  virtual ~DieselScreen();

  lv_obj_t* getScreen() const { return _screen; }

  virtual void onLoad();
  virtual void onUnload();
  virtual void onTimer();
  virtual void onBack();
  virtual void onSettings() {}
  void setReturnScreen(DieselScreen* s) { _returnScreen = s; }

  static void initTheme();
  static void setNavGroup(lv_group_t* group);

protected:
  // Navigation
  void pushScreen(DieselScreen* screen);
  void popScreen();
  void showHome();

  // Header bar helpers
  lv_obj_t* createHeader(lv_obj_t* parent);
  void updateHeaderClock();
  void updateHeaderBtIcon(bool connected);
  void updateHeaderWifiIcon(int rssi);
  void updateHeaderBattery(float volts);
  void updateHeaderHeaterState(int state);
  void hideBackButton();

  // Content area (scrollable container below header)
  lv_obj_t* createContentArea();

  // Labels with theme styling
  lv_obj_t* createLabel(lv_obj_t* parent, const char* text, lv_align_t align, lv_coord_t x, lv_coord_t y);
  lv_obj_t* createBigLabel(lv_obj_t* parent, const char* text, lv_align_t align, lv_coord_t x, lv_coord_t y);
  lv_obj_t* createValueLabel(lv_obj_t* parent, const char* text, lv_align_t align, lv_coord_t x, lv_coord_t y);
  lv_obj_t* createUnitLabel(lv_obj_t* parent, const char* text, lv_align_t align, lv_coord_t x, lv_coord_t y);

  // Status indicators (colored dots)
  lv_obj_t* createLed(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_color_t color);

  // Gauge bar
  lv_obj_t* createBar(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, int16_t range_max);

  // Screen navigation buttons
  lv_obj_t* createNavButton(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y);

  const char* _name;
  lv_obj_t* _screen = nullptr;
  lv_obj_t* _header = nullptr;
  lv_obj_t* _headerClock = nullptr;
  lv_obj_t* _headerBack = nullptr;
  lv_obj_t* _headerSettings = nullptr;
  lv_obj_t* _headerBt = nullptr;
  lv_obj_t* _headerWifi = nullptr;
  lv_obj_t* _headerBattery = nullptr;
  lv_obj_t* _headerHeater = nullptr;
  lv_timer_t* _timer = nullptr;

  DieselScreen* _returnScreen = nullptr;
  static lv_group_t* _navGroup;
};
