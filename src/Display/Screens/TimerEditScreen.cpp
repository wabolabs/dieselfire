#include "TimerEditScreen.h"
#include "../../RTC/TimerManager.h"
#include "../../Utility/NVStorage.h"
#include "../../cfg/DFConfig.h"

extern CHeaterStorage& NVstore;

static const char* DAY_NAMES[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const int ROLLER_H = 44;
static const int LINE_H = 34;

TimerEditScreen::TimerEditScreen() : DieselScreen("TimerEdit") {}

static lv_obj_t* makeRoller(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w,
                             const char* opts, int sel, lv_event_cb_t cb, void* udata) {
  lv_obj_t* r = lv_roller_create(parent);
  lv_obj_set_pos(r, x, y);
  lv_obj_set_size(r, w, ROLLER_H);
  lv_roller_set_options(r, opts, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_selected(r, sel, LV_ANIM_OFF);
  lv_obj_add_event_cb(r, cb, LV_EVENT_VALUE_CHANGED, udata);
  return r;
}

void TimerEditScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Timer", LV_ALIGN_TOP_LEFT, 48, 3);

  lv_obj_t* cont = createContentArea();
  int x = 4, y = 4;

  // Row 1: Timer selector
  createLabel(cont, "Timer #", LV_ALIGN_TOP_LEFT, x, y + 8);
  _timerSelector = makeRoller(cont, 70, y, 60,
    "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14", 0, onTimerChange, this);

  // Row 2: Start time
  y += LINE_H + ROLLER_H;
  createLabel(cont, "Start", LV_ALIGN_TOP_LEFT, x, y + 8);
  _startH = makeRoller(cont, 50, y, 48,
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23", 0, onTimerChange, this);
  createLabel(cont, ":", LV_ALIGN_TOP_LEFT, 100, y + 10);
  _startM = makeRoller(cont, 108, y, 48,
    "00\n05\n10\n15\n20\n25\n30\n35\n40\n45\n50\n55", 0, onTimerChange, this);

  // Row 3: Stop time
  y += LINE_H + ROLLER_H;
  createLabel(cont, "Stop", LV_ALIGN_TOP_LEFT, x, y + 8);
  _stopH = makeRoller(cont, 50, y, 48,
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23", 0, onTimerChange, this);
  createLabel(cont, ":", LV_ALIGN_TOP_LEFT, 100, y + 10);
  _stopM = makeRoller(cont, 108, y, 48,
    "00\n05\n10\n15\n20\n25\n30\n35\n40\n45\n50\n55", 0, onTimerChange, this);

  // Row 4: Days of week
  y += LINE_H + ROLLER_H;
  createLabel(cont, "Days", LV_ALIGN_TOP_LEFT, x, y + 2);
  for (int i = 0; i < 7; i++) {
    _dayBtns[i] = lv_button_create(cont);
    lv_obj_set_pos(_dayBtns[i], 48 + i * 34, y);
    lv_obj_set_size(_dayBtns[i], 30, 24);
    lv_obj_set_style_bg_color(_dayBtns[i], lv_color_make(0x33,0x33,0x33), 0);
    lv_obj_set_style_bg_color(_dayBtns[i], lv_color_make(0xFF,0x8C,0x00), LV_STATE_CHECKED);
    lv_obj_add_flag(_dayBtns[i], LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_border_width(_dayBtns[i], 0, 0);
    lv_obj_t* lbl = lv_label_create(_dayBtns[i]);
    lv_label_set_text(lbl, DAY_NAMES[i]);
    lv_obj_center(lbl);
    lv_obj_add_event_cb(_dayBtns[i], onTimerChange, LV_EVENT_VALUE_CHANGED, this);
  }

  // Row 5: Repeat toggle
  y += LINE_H + 24;
  createLabel(cont, "Repeat", LV_ALIGN_TOP_LEFT, x, y + 2);
  _repeatSwitch = lv_switch_create(cont);
  lv_obj_set_pos(_repeatSwitch, 60, y);

  // Row 6: Temperature
  y += LINE_H + 24;
  createLabel(cont, "Temp", LV_ALIGN_TOP_LEFT, x, y + 2);
  _tempSlider = lv_slider_create(cont);
  lv_obj_set_pos(_tempSlider, 50, y);
  lv_obj_set_size(_tempSlider, TFT_WIDTH - 110, 16);
  lv_slider_set_range(_tempSlider, 80, 350);
  lv_obj_add_event_cb(_tempSlider, onTimerChange, LV_EVENT_VALUE_CHANGED, this);
  _tempLabel = lv_label_create(cont);
  lv_obj_align_to(_tempLabel, _tempSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

  // Set content height for scrolling
  lv_obj_set_height(cont, y + 50);
  loadTimer(0);
}

void TimerEditScreen::onTimerChange(lv_event_t* e) {
  static_cast<TimerEditScreen*>(lv_event_get_user_data(e))->saveTimer();
}

void TimerEditScreen::loadTimer(int idx) {
  _timerIdx = idx;
  sTimer t;
  t.init(idx);
  NVstore.getTimerInfo(idx, t);

  lv_roller_set_selected(_startH, t.start.hour, LV_ANIM_OFF);
  lv_roller_set_selected(_startM, t.start.min / 5, LV_ANIM_OFF);
  lv_roller_set_selected(_stopH, t.stop.hour, LV_ANIM_OFF);
  lv_roller_set_selected(_stopM, t.stop.min / 5, LV_ANIM_OFF);

  for (int d = 0; d < 7; d++) {
    if (t.enabled & (1 << d))
      lv_obj_add_state(_dayBtns[d], LV_STATE_CHECKED);
    else
      lv_obj_clear_state(_dayBtns[d], LV_STATE_CHECKED);
  }

  if (t.repeat)
    lv_obj_add_state(_repeatSwitch, LV_STATE_CHECKED);
  else
    lv_obj_clear_state(_repeatSwitch, LV_STATE_CHECKED);

  lv_slider_set_value(_tempSlider, t.temperature * 10, LV_ANIM_OFF);
  updateLabels();
}

void TimerEditScreen::saveTimer() {
  sTimer t;
  t.init(_timerIdx);
  t.start.hour = lv_roller_get_selected(_startH);
  t.start.min = lv_roller_get_selected(_startM) * 5;
  t.stop.hour = lv_roller_get_selected(_stopH);
  t.stop.min = lv_roller_get_selected(_stopM) * 5;
  t.repeat = lv_obj_has_state(_repeatSwitch, LV_STATE_CHECKED) ? 1 : 0;
  t.temperature = lv_slider_get_value(_tempSlider) / 10;

  uint8_t days = 0;
  for (int d = 0; d < 7; d++) {
    if (lv_obj_has_state(_dayBtns[d], LV_STATE_CHECKED))
      days |= (1 << d);
  }
  t.enabled = days;

  NVstore.setTimerInfo(_timerIdx, t);
  NVstore.save();
  updateLabels();
}

void TimerEditScreen::updateLabels() {
  char buf[8];
  snprintf(buf, sizeof(buf), "%d\xC2\xB0", lv_slider_get_value(_tempSlider) / 10);
  lv_label_set_text(_tempLabel, buf);
}
