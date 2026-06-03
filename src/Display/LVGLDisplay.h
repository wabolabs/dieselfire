#pragma once

#include <lvgl.h>

class LVGLDisplay {
public:
  void begin();
  void taskHandler();
  void showMainScreen();

private:
  static void flushCb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map);
  static void touchReadCb(lv_indev_t* indev, lv_indev_data_t* data);

  lv_display_t* _disp = nullptr;
  lv_indev_t* _indev = nullptr;
  uint8_t* _buf1 = nullptr;
  uint8_t* _buf2 = nullptr;
};
