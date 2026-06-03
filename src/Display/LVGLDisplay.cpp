#include "LVGLDisplay.h"
#include "ILI9341Driver.h"
#include "Screens/DieselScreen.h"
#include "Screens/MainStatusScreen.h"
#include "../cfg/pins.h"
#include "../cfg/DFConfig.h"
#include "../Utility/DebugPort.h"

static ILI9341Driver panel;
static MainStatusScreen* mainScreen = nullptr;

void LVGLDisplay::begin() {
  DebugPort.println("LVGL: starting");

  panel.init();
  panel.setRotation(1);
  panel.setBrightness(200);
  panel.fillScreen(TFT_BLACK);
  DebugPort.println("Display ready");

  lv_init();
  DieselScreen::initTheme();

  size_t bufSize = TFT_WIDTH * TFT_HEIGHT * 2 / 10;
  _buf1 = (uint8_t*)ps_malloc(bufSize);
  _buf2 = (uint8_t*)ps_malloc(bufSize);
  if (!_buf1 || !_buf2) {
    _buf1 = (uint8_t*)malloc(bufSize);
    _buf2 = (uint8_t*)malloc(bufSize);
  }
  DebugPort.printf("LVGL buffer: %u bytes x2\r\n", bufSize);

  _disp = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
  lv_display_set_flush_cb(_disp, flushCb);
  lv_display_set_user_data(_disp, &panel);
  lv_display_set_buffers(_disp, _buf1, _buf2, bufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

  _indev = lv_indev_create();
  lv_indev_set_type(_indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(_indev, touchReadCb);

  mainScreen = new MainStatusScreen();
  mainScreen->onLoad();
  lv_scr_load(mainScreen->getScreen());

  DebugPort.println("LVGL ready");
}

void LVGLDisplay::showMainScreen() {
  if (mainScreen) {
    lv_scr_load(mainScreen->getScreen());
  }
}

void LVGLDisplay::taskHandler() {
  static uint32_t lastTick = 0;
  uint32_t now = millis();
  if (now - lastTick >= 5) {
    lv_tick_inc(now - lastTick);
    lastTick = now;
  }
  lv_task_handler();
}

void LVGLDisplay::flushCb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  auto* p = static_cast<ILI9341Driver*>(lv_display_get_user_data(disp));
  auto x1 = area->x1, y1 = area->y1;
  auto w = lv_area_get_width(area);
  auto h = lv_area_get_height(area);
  p->pushImageDMA(x1, y1, w, h, (lgfx::swap565_t*)px_map);
  lv_display_flush_ready(disp);
}

void LVGLDisplay::touchReadCb(lv_indev_t* indev, lv_indev_data_t* data) {
  lgfx::touch_point_t tp;
  if (panel.getTouch(&tp)) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = tp.x;
    data->point.y = tp.y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}
