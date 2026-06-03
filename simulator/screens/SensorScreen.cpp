#include <cstdio>
#include "SensorScreen.h"
#include "Utility/TempSense.h"
#include "Utility/DataFilter.h"

extern CTempSense TempSensor;
extern sFilteredData FilteredSamples;

SensorScreen::SensorScreen() : DieselScreen("Sensors") {}

void SensorScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Sensors", LV_ALIGN_TOP_LEFT, 48, 3);

  int y = 40;
  createLabel(_screen, "Ambient", LV_ALIGN_TOP_LEFT, 8, y);
  _tempLabel = createValueLabel(_screen, "--.- \xC2\xB0C", LV_ALIGN_TOP_LEFT, 70, y);

  y += 30;
  createLabel(_screen, "Humidity", LV_ALIGN_TOP_LEFT, 8, y);
  _humLabel = createValueLabel(_screen, "--.- %", LV_ALIGN_TOP_LEFT, 70, y);

  y += 30;
  createLabel(_screen, "Altitude", LV_ALIGN_TOP_LEFT, 8, y);
  _altLabel = createValueLabel(_screen, "--- m", LV_ALIGN_TOP_LEFT, 70, y);

  y += 30;
  createLabel(_screen, "External", LV_ALIGN_TOP_LEFT, 8, y);
  _extTempLabel = createValueLabel(_screen, "--.- \xC2\xB0C", LV_ALIGN_TOP_LEFT, 70, y);

  _timer = lv_timer_create([](lv_timer_t* t) {
    static_cast<SensorScreen*>(lv_timer_get_user_data(t))->onTimer();
  }, 1000, this);
}

void SensorScreen::onTimer() { updateData(); }

void SensorScreen::updateData() {
  char buf[32];

  float temp = FilteredSamples.AmbientTemp.getValue();
  if (temp > -50) {
    snprintf(buf, sizeof(buf), "%.1f \xC2\xB0C", temp);
    lv_label_set_text(_tempLabel, buf);
  }

  float hum;
  if (TempSensor.getHumidity(hum)) {
    snprintf(buf, sizeof(buf), "%.0f %%", hum);
    lv_label_set_text(_humLabel, buf);
  }

  float alt;
  if (TempSensor.getAltitude(alt)) {
    snprintf(buf, sizeof(buf), "%.0f m", alt);
    lv_label_set_text(_altLabel, buf);
  }

  float extTemp;
  if (TempSensor.getTemperature(0, extTemp)) {
    snprintf(buf, sizeof(buf), "%.1f \xC2\xB0C", extTemp);
    lv_label_set_text(_extTempLabel, buf);
  }
}
