#include "GPIOScreen.h"
#include "Utility/DF_GPIO.h"
#include "Utility/NVStorage.h"

extern CHeaterStorage& NVstore;
extern CGPIOin GPIOin;
extern CGPIOout GPIOout;
extern CGPIOalg GPIOalg;

GPIOScreen::GPIOScreen() : DieselScreen("GPIO") {}

void GPIOScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "GPIO", LV_ALIGN_TOP_LEFT, 48, 3);

  int y = 36;
  createLabel(_screen, "In 1", LV_ALIGN_TOP_LEFT, 8, y);
  _in1Label = createValueLabel(_screen, "--/--", LV_ALIGN_TOP_LEFT, 60, y);

  y += 28;
  createLabel(_screen, "In 2", LV_ALIGN_TOP_LEFT, 8, y);
  _in2Label = createValueLabel(_screen, "--/--", LV_ALIGN_TOP_LEFT, 60, y);

  y += 28;
  createLabel(_screen, "Out 1", LV_ALIGN_TOP_LEFT, 8, y);
  _out1Label = createValueLabel(_screen, "--/--", LV_ALIGN_TOP_LEFT, 60, y);

  y += 28;
  createLabel(_screen, "Out 2", LV_ALIGN_TOP_LEFT, 8, y);
  _out2Label = createValueLabel(_screen, "--/--", LV_ALIGN_TOP_LEFT, 60, y);

  y += 28;
  createLabel(_screen, "Analog", LV_ALIGN_TOP_LEFT, 8, y);
  _analogLabel = createValueLabel(_screen, "-- %", LV_ALIGN_TOP_LEFT, 60, y);

  _timer = lv_timer_create([](lv_timer_t* t) {
    static_cast<GPIOScreen*>(lv_timer_get_user_data(t))->onTimer();
  }, 500, this);
}

void GPIOScreen::onTimer() { updateData(); }

void GPIOScreen::updateData() {
  char buf[24];
  snprintf(buf, sizeof(buf), "%d %s", GPIOin.getState(0),
           GPIOin.getState(0) ? "HIGH" : "LOW");
  lv_label_set_text(_in1Label, buf);

  snprintf(buf, sizeof(buf), "%d %s", GPIOin.getState(1),
           GPIOin.getState(1) ? "HIGH" : "LOW");
  lv_label_set_text(_in2Label, buf);

  snprintf(buf, sizeof(buf), "%d %s", GPIOout.getState(0),
           GPIOout.getState(0) ? "ON" : "OFF");
  lv_label_set_text(_out1Label, buf);

  snprintf(buf, sizeof(buf), "%d %s", GPIOout.getState(1),
           GPIOout.getState(1) ? "ON" : "OFF");
  lv_label_set_text(_out2Label, buf);

  snprintf(buf, sizeof(buf), "%d %%", GPIOalg.getValue());
  lv_label_set_text(_analogLabel, buf);
}
