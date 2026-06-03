#include "cfg/DFConfig.h"
#include "TimerChartScreen.h"
#include "cfg/DFConfig.h"
#include "RTC/TimerManager.h"
#include "RTC/Clock.h"
#include "Utility/NVStorage.h"
#include <cstdio>


static const char* DAY_LABELS[7] = {"S","M","T","W","T","F","S"};

TimerChartScreen::TimerChartScreen() : DieselScreen("TimerChart") {}

void TimerChartScreen::onLoad() {
  createHeader(_screen);

  // Scrollable list of timers
  lv_obj_t* list = lv_obj_create(_screen);
  lv_obj_set_size(list, TFT_WIDTH, TFT_HEIGHT - 22);
  lv_obj_set_pos(list, 0, 22);
  lv_obj_set_style_bg_color(list, lv_color_make(0x12, 0x12, 0x12), 0);
  lv_obj_set_style_border_width(list, 0, 0);
  lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

  char buf[48];
  for (int idx = 0; idx < 14; idx++) {
    sTimer t;
    t.init(idx);
    NVstore.getTimerInfo(idx, t);

    // Container for one timer row
    lv_obj_t* row = lv_obj_create(list);
    lv_obj_set_size(row, TFT_WIDTH - 8, 24);
    lv_obj_set_style_bg_color(row, lv_color_make(0x1E, 0x1E, 0x1E), 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 2, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

    // Timer number
    snprintf(buf, sizeof(buf), "%2d", idx + 1);
    lv_obj_t* num = lv_label_create(row);
    lv_label_set_text(num, buf);
    lv_obj_set_style_text_color(num, lv_palette_main(LV_PALETTE_ORANGE), 0);
    lv_obj_set_width(num, 20);

    // Day-of-week dots
    for (int d = 0; d < 7; d++) {
      lv_obj_t* dot = lv_label_create(row);
      lv_label_set_text(dot, (t.enabled & (1 << d)) ? DAY_LABELS[d] : "_");
      lv_obj_set_style_text_color(dot,
        (t.enabled & (1 << d)) ? lv_color_white() : lv_color_make(0x44,0x44,0x44), 0);
      lv_obj_set_width(dot, 14);
    }

    // Time range
    snprintf(buf, sizeof(buf), "%02d:%02d-%02d:%02d",
             t.start.hour, t.start.min, t.stop.hour, t.stop.min);
    lv_obj_t* time_lbl = lv_label_create(row);
    lv_label_set_text(time_lbl, buf);
    lv_obj_set_style_text_color(time_lbl, lv_color_white(), 0);
    lv_obj_set_width(time_lbl, 80);

    // Repeat indicator
    if (t.repeat) {
      lv_obj_t* rpt = lv_label_create(row);
      lv_label_set_text(rpt, "R");
      lv_obj_set_style_text_color(rpt, lv_color_make(0x00,0xCC,0x00), 0);
    }

    // Temperature
    snprintf(buf, sizeof(buf), "%d\xC2\xB0", t.temperature);
    lv_obj_t* temp = lv_label_create(row);
    lv_label_set_text(temp, buf);
    lv_obj_set_style_text_color(temp, lv_color_make(0x88,0x88,0x88), 0);
  }
}
