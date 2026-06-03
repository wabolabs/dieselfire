#include "MainStatusScreen.h"
#include "SettingsMenuScreen.h"
#include "Utility/DataFilter.h"
#include "Protocol/Protocol.h"
#include "RTC/Clock.h"
#include "Utility/NVStorage.h"
#include "Utility/FuelGauge.h"
#include "Utility/DemandManager.h"
#include "cfg/DFConfig.h"

// External globals from main.cpp
extern CProtocolPackage BlueWireData;
extern CFuelGauge FuelGauge;
extern sFilteredData FilteredSamples;
extern const CProtocolPackage& getHeaterInfo();

static const CProtocolPackage& hi() { return getHeaterInfo(); }

static void mainTimerCb(lv_timer_t* t) {
  auto* s = static_cast<MainStatusScreen*>(lv_timer_get_user_data(t));
  if (s) s->onTimer();
}

MainStatusScreen::MainStatusScreen()
  : DieselScreen("MainStatus") {}

void MainStatusScreen::onLoad() {
  createHeader(_screen);
  _tabview = lv_tabview_create(_screen);
  lv_obj_set_size(_tabview, TFT_WIDTH, TFT_HEIGHT - 22);
  lv_obj_set_pos(_tabview, 0, 22);
  lv_obj_set_style_bg_color(_tabview, lv_color_make(0x12, 0x12, 0x12), 0);

  lv_obj_t* tabTelem = lv_tabview_add_tab(_tabview, "Data");
  lv_obj_t* tabTemp  = lv_tabview_add_tab(_tabview, "Temp");
  lv_obj_t* tabClock = lv_tabview_add_tab(_tabview, "Time");
  lv_obj_t* tabPrime = lv_tabview_add_tab(_tabview, "Fuel");

  lv_obj_set_style_bg_color(tabTelem, lv_color_make(0x12, 0x12, 0x12), 0);
  lv_obj_set_style_bg_color(tabTemp, lv_color_make(0x12, 0x12, 0x12), 0);
  lv_obj_set_style_bg_color(tabClock, lv_color_make(0x12, 0x12, 0x12), 0);
  lv_obj_set_style_bg_color(tabPrime, lv_color_make(0x12, 0x12, 0x12), 0);
  lv_obj_set_scrollbar_mode(tabTelem, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scrollbar_mode(tabTemp, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scrollbar_mode(tabClock, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scrollbar_mode(tabPrime, LV_SCROLLBAR_MODE_OFF);

  buildTelemetryTab(tabTelem);
  buildBigTempTab(tabTemp);
  buildClockTab(tabClock);
  buildPrimingTab(tabPrime);

  _timer = lv_timer_create(mainTimerCb, 200, this);
}

// ── Telemetry tab ───────────────────────────────────────────
void MainStatusScreen::buildTelemetryTab(lv_obj_t* parent) {
  createLabel(parent, "Ambient", LV_ALIGN_TOP_LEFT, 8, 8);
  _telemTemp = createBigLabel(parent, "--.-", LV_ALIGN_TOP_LEFT, 8, 22);
  createUnitLabel(parent, "\xEF\x81\xB7", LV_ALIGN_TOP_LEFT, 90, 28);

  createLabel(parent, "Pump", LV_ALIGN_TOP_LEFT, 8, 70);
  _telemPump = createValueLabel(parent, "--.- Hz", LV_ALIGN_TOP_LEFT, 8, 86);

  createLabel(parent, "RPM", LV_ALIGN_TOP_RIGHT, -8, 8);
  _telemRpm = createValueLabel(parent, "----", LV_ALIGN_TOP_RIGHT, -8, 22);
  createUnitLabel(parent, "RPM", LV_ALIGN_TOP_RIGHT, -8, 44);

  createLabel(parent, "Glow", LV_ALIGN_TOP_RIGHT, -8, 70);
  _telemGlow = createValueLabel(parent, "--.- W", LV_ALIGN_TOP_RIGHT, -8, 86);

  _telemState = createLabel(parent, "Off", LV_ALIGN_BOTTOM_LEFT, 8, -8);
  _telemFuel = createValueLabel(parent, "0.0L", LV_ALIGN_BOTTOM_RIGHT, -8, -8);
}

void MainStatusScreen::updateTelemetry() {
  char buf[32];
  int state = hi().getRunState();

  // State
  const char* stateStr = "Off";
  if (state == 0) stateStr = hi().getErrState() > 0 ? "Error" : "Standby";
  else if (state == 1) stateStr = "Starting";
  else if (state == 2) stateStr = "GlowOn";
  else if (state == 3) stateStr = "Ignited";
  else if (state == 4) stateStr = "Running";
  else if (state == 5) stateStr = "Shutdown";
  lv_label_set_text(_telemState, stateStr);

  // Temperature
  float temp = FilteredSamples.AmbientTemp.getValue();
  if (temp > -50) {
    snprintf(buf, sizeof(buf), "%.1f", temp);
    lv_label_set_text(_telemTemp, buf);
  }

  // Pump
  snprintf(buf, sizeof(buf), "%.1f Hz", hi().getPump_Actual());
  lv_label_set_text(_telemPump, buf);

  // Fan
  snprintf(buf, sizeof(buf), "%d", hi().getFan_Actual());
  lv_label_set_text(_telemRpm, buf);

  // Glow
  snprintf(buf, sizeof(buf), "%.1f W", hi().getGlowPlug_Power());
  lv_label_set_text(_telemGlow, buf);

  // Fuel
  snprintf(buf, sizeof(buf), "%.1fL", FuelGauge.Used_mL() / 1000.0f);
  lv_label_set_text(_telemFuel, buf);

  // Header
  updateHeaderHeaterState(state);
  updateHeaderBattery(BlueWireData.getBattVoltage());
}

// ── Big temperature tab ─────────────────────────────────────
void MainStatusScreen::buildBigTempTab(lv_obj_t* parent) {
  _bigTemp = createBigLabel(parent, "--.-", LV_ALIGN_CENTER, 0, -24);
  lv_obj_set_style_text_font(_bigTemp, &lv_font_montserrat_36, 0);
  createUnitLabel(parent, "\xC2\xB0", LV_ALIGN_CENTER, 90, -30);
  _bigState = createLabel(parent, "", LV_ALIGN_CENTER, 0, 28);
}

void MainStatusScreen::updateBigTemp() {
  char buf[16];
  float temp = FilteredSamples.AmbientTemp.getValue();
  if (temp > -50) {
    snprintf(buf, sizeof(buf), "%.1f", temp);
    lv_label_set_text(_bigTemp, buf);
  }
  int state = hi().getRunState();
  if (state == 0 && hi().getErrState() > 0) {
    snprintf(buf, sizeof(buf), "E-%02d", hi().getErrState());
  } else {
    snprintf(buf, sizeof(buf), "%.1f", CDemandManager::getDegC());
  }
  lv_label_set_text(_bigState, buf);
}

// ── Clock tab ───────────────────────────────────────────────
void MainStatusScreen::buildClockTab(lv_obj_t* parent) {
  _clockDisplay = createBigLabel(parent, "00:00", LV_ALIGN_CENTER, 0, -24);
  _clockDate = createLabel(parent, "", LV_ALIGN_CENTER, 0, 24);
}

void MainStatusScreen::updateClockTabContent() {
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d", Clock.get().hour(), Clock.get().minute());
  lv_label_set_text(_clockDisplay, buf);

  static const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  int d = Clock.get().dayOfTheWeek();
  snprintf(buf, sizeof(buf), "%s %02d/%02d/%04d",
           days[d], Clock.get().day(), Clock.get().month(), Clock.get().year());
  lv_label_set_text(_clockDate, buf);
}

// ── Priming tab ─────────────────────────────────────────────
void MainStatusScreen::buildPrimingTab(lv_obj_t* parent) {
  // TODO: priming controls
  _primeStatus = createLabel(parent, "Priming not active", LV_ALIGN_CENTER, 0, -20);
  createLabel(parent, "Fuel used:", LV_ALIGN_CENTER, 0, 10);
  _primeFuelUsed = createValueLabel(parent, "0.0 L", LV_ALIGN_CENTER, 0, 30);
}

void MainStatusScreen::onSettings() {
  auto* s = new SettingsMenuScreen();
  s->onLoad();
  lv_scr_load(s->getScreen());
}

void MainStatusScreen::updatePriming() {
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f L", FuelGauge.Used_mL() / 1000.0f);
  lv_label_set_text(_primeFuelUsed, buf);
}

// ── Timer callback ──────────────────────────────────────────
void MainStatusScreen::onTimer() {
  updateHeaderClock();
  updateTelemetry();
  updateBigTemp();
  updateClockTabContent();
  updatePriming();
}
