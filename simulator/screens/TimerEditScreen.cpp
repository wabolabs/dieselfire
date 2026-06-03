#include "cfg/DFConfig.h"
#include "TimerEditScreen.h"
#include "cfg/DFConfig.h"
#include "RTC/TimerManager.h"
#include "Utility/NVStorage.h"


static const char* DAY_NAMES[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

TimerEditScreen::TimerEditScreen() : DieselScreen("TimerEdit") {}

void TimerEditScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Timer", LV_ALIGN_TOP_LEFT, 48, 3);

  // Timer selector (1-14)
  createLabel(_screen, "Timer #", LV_ALIGN_TOP_LEFT, 8, 28);
  _timerSelector = lv_roller_create(_screen);
  lv_obj_set_pos(_timerSelector, 60, 28);
  lv_obj_set_width(_timerSelector, 60);
  lv_roller_set_options(_timerSelector,
    "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14", LV_ROLLER_MODE_NORMAL);
  lv_roller_set_selected(_timerSelector, 0, LV_ANIM_OFF);
  lv_obj_add_event_cb(_timerSelector, onTimerChange, LV_EVENT_VALUE_CHANGED, this);

  // Start time
  int y = 70;
  createLabel(_screen, "Start", LV_ALIGN_TOP_LEFT, 8, y);
  _startH = lv_roller_create(_screen);
  lv_obj_set_pos(_startH, 50, y);
  lv_obj_set_width(_startH, 50);
  lv_roller_set_options(_startH, "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23", LV_ROLLER_MODE_NORMAL);
  lv_obj_add_event_cb(_startH, onTimerChange, LV_EVENT_VALUE_CHANGED, this);

  lv_obj_t* colon = createLabel(_screen, ":", LV_ALIGN_TOP_LEFT, 103, y + 4);

  _startM = lv_roller_create(_screen);
  lv_obj_set_pos(_startM, 112, y);
  lv_obj_set_width(_startM, 50);
  lv_roller_set_options(_startM, "00\n05\n10\n15\n20\n25\n30\n35\n40\n45\n50\n55", LV_ROLLER_MODE_NORMAL);
  lv_obj_add_event_cb(_startM, onTimerChange, LV_EVENT_VALUE_CHANGED, this);

  // Stop time
  y += 44;
  createLabel(_screen, "Stop", LV_ALIGN_TOP_LEFT, 8, y);
  _stopH = lv_roller_create(_screen);
  lv_obj_set_pos(_stopH, 50, y);
  lv_obj_set_width(_stopH, 50);
  lv_roller_set_options(_stopH, "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23", LV_ROLLER_MODE_NORMAL);
  lv_obj_add_event_cb(_stopH, onTimerChange, LV_EVENT_VALUE_CHANGED, this);

  createLabel(_screen, ":", LV_ALIGN_TOP_LEFT, 103, y + 4);

  _stopM = lv_roller_create(_screen);
  lv_obj_set_pos(_stopM, 112, y);
  lv_obj_set_width(_stopM, 50);
  lv_roller_set_options(_stopM, "00\n05\n10\n15\n20\n25\n30\n35\n40\n45\n50\n55", LV_ROLLER_MODE_NORMAL);
  lv_obj_add_event_cb(_stopM, onTimerChange, LV_EVENT_VALUE_CHANGED, this);

  // Days of week
  y += 44;
  createLabel(_screen, "Days", LV_ALIGN_TOP_LEFT, 8, y);
  for (int i = 0; i < 7; i++) {
    _dayBtns[i] = lv_button_create(_screen);
    lv_obj_set_pos(_dayBtns[i], 48 + i * 38, y);
    lv_obj_set_size(_dayBtns[i], 34, 22);
    lv_obj_set_style_bg_color(_dayBtns[i], lv_color_make(0x33,0x33,0x33), 0);
    lv_obj_set_style_bg_color(_dayBtns[i], lv_color_make(0xFF,0x8C,0x00), LV_STATE_CHECKED);
    lv_obj_add_flag(_dayBtns[i], LV_OBJ_FLAG_CHECKABLE);
    lv_obj_t* lbl = lv_label_create(_dayBtns[i]);
    lv_label_set_text(lbl, DAY_NAMES[i]);
    lv_obj_center(lbl);
    lv_obj_add_event_cb(_dayBtns[i], onTimerChange, LV_EVENT_VALUE_CHANGED, this);
  }

  // Repeat toggle
  y += 30;
  createLabel(_screen, "Repeat", LV_ALIGN_TOP_LEFT, 8, y);
  _repeatSwitch = lv_switch_create(_screen);
  lv_obj_set_pos(_repeatSwitch, 60, y);
  lv_obj_add_event_cb(_repeatSwitch, onTimerChange, LV_EVENT_VALUE_CHANGED, this);

  // Temperature
  y += 34;
  createLabel(_screen, "Temp", LV_ALIGN_TOP_LEFT, 8, y);
  _tempSlider = lv_slider_create(_screen);
  lv_obj_set_pos(_tempSlider, 50, y);
  lv_obj_set_size(_tempSlider, TFT_WIDTH - 110, 16);
  lv_slider_set_range(_tempSlider, 80, 350);
  lv_obj_add_event_cb(_tempSlider, onTimerChange, LV_EVENT_VALUE_CHANGED, this);
  _tempLabel = lv_label_create(_screen);
  lv_obj_align_to(_tempLabel, _tempSlider, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

  loadTimer(0);
}

void TimerEditScreen::onTimerChange(lv_event_t* e) {
  auto* self = static_cast<TimerEditScreen*>(lv_event_get_user_data(e));
  self->saveTimer();
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
