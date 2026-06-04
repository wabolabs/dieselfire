#include "MainStatusScreen.h"
#include "SettingsMenuScreen.h"
#include "Protocol.h"
#include "RTC/Clock.h"
#include "Utility/NVStorage.h"
#include "Utility/DataFilter.h"
#include "Utility/FuelGauge.h"
#include "Utility/DemandManager.h"
#include "Utility/helpers.h"
#include "cfg/DFConfig.h"

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

  // Heater ON/OFF button
  _heaterBtn = lv_button_create(parent);
  lv_obj_set_pos(_heaterBtn, 8, 104);
  lv_obj_set_size(_heaterBtn, TFT_WIDTH - 16, 30);
  lv_obj_set_style_bg_color(_heaterBtn, lv_color_make(0xFF,0x8C,0x00), 0);
  lv_obj_set_style_bg_color(_heaterBtn, lv_color_make(0xCC,0x44,0x00), LV_STATE_PRESSED);
  lv_obj_set_style_border_width(_heaterBtn, 0, 0);
  lv_obj_t* lbl = lv_label_create(_heaterBtn);
  lv_label_set_text(lbl, "START");
  lv_obj_center(lbl);
  lv_obj_set_style_text_color(lbl, lv_color_make(0,0,0), 0);
  lv_obj_add_event_cb(_heaterBtn, onHeaterBtn, LV_EVENT_CLICKED, this);
}

void MainStatusScreen::updateTelemetry() {
  char buf[32];
  int state = hi().getRunState();

  const char* stateStr = "Off";
  if (state == 0) stateStr = hi().getErrState() > 0 ? "Error" : "Standby";
  else if (state == 1) stateStr = "Starting";
  else if (state == 2) stateStr = "GlowOn";
  else if (state == 3) stateStr = "Ignited";
  else if (state == 4) stateStr = "Running";
  else if (state == 5) stateStr = "Shutdown";
  lv_label_set_text(_telemState, stateStr);

  float temp = FilteredSamples.AmbientTemp.getValue();
  if (temp > -50) {
    snprintf(buf, sizeof(buf), "%.1f", temp);
    lv_label_set_text(_telemTemp, buf);
  }

  snprintf(buf, sizeof(buf), "%.1f Hz", hi().getPump_Actual());
  lv_label_set_text(_telemPump, buf);

  snprintf(buf, sizeof(buf), "%d", hi().getFan_Actual());
  lv_label_set_text(_telemRpm, buf);

  snprintf(buf, sizeof(buf), "%.1f W", hi().getGlowPlug_Power());
  lv_label_set_text(_telemGlow, buf);

  snprintf(buf, sizeof(buf), "%.1fL", FuelGauge.Used_mL() / 1000.0f);
  lv_label_set_text(_telemFuel, buf);

  updateHeaderHeaterState(state);
  updateHeaderBattery(BlueWireData.getBattVoltage());
  updateHeaterButton();
}

void MainStatusScreen::updateHeaterButton() {
  int state = hi().getRunState();
  bool running = state > 0;
  lv_obj_t* lbl = lv_obj_get_child(_heaterBtn, 0);
  if (!lbl) return;
  lv_label_set_text(lbl, running ? "STOP" : "START");
  lv_obj_set_style_bg_color(_heaterBtn,
    running ? lv_color_make(0xCC,0x33,0x00) : lv_color_make(0xFF,0x8C,0x00), 0);
}

void MainStatusScreen::onHeaterBtn(lv_event_t* e) {
  int state = hi().getRunState();
  if (state > 0) requestOff();
  else requestOn();
}

// ── Big temperature tab ─────────────────────────────────────
void MainStatusScreen::buildBigTempTab(lv_obj_t* parent) {
  _bigTemp = createBigLabel(parent, "--.-", LV_ALIGN_CENTER, 0, -36);
  lv_obj_set_style_text_font(_bigTemp, &lv_font_montserrat_36, 0);

  // Down arrow
  _tempDown = lv_button_create(parent);
  lv_obj_set_pos(_tempDown, 40, 10);
  lv_obj_set_size(_tempDown, 36, 36);
  lv_obj_set_style_bg_color(_tempDown, lv_color_make(0x44,0x44,0x44), 0);
  lv_obj_set_style_border_width(_tempDown, 0, 0);
  lv_obj_t* dlbl = lv_label_create(_tempDown);
  lv_label_set_text(dlbl, "-");
  lv_obj_center(dlbl);
  lv_obj_set_style_text_font(dlbl, &lv_font_montserrat_36, 0);
  lv_obj_add_event_cb(_tempDown, onTempDown, LV_EVENT_CLICKED, this);

  // Up arrow
  _tempUp = lv_button_create(parent);
  lv_obj_set_pos(_tempUp, 240, 10);
  lv_obj_set_size(_tempUp, 36, 36);
  lv_obj_set_style_bg_color(_tempUp, lv_color_make(0x44,0x44,0x44), 0);
  lv_obj_set_style_border_width(_tempUp, 0, 0);
  lv_obj_t* ulbl = lv_label_create(_tempUp);
  lv_label_set_text(ulbl, "+");
  lv_obj_center(ulbl);
  lv_obj_set_style_text_font(ulbl, &lv_font_montserrat_36, 0);
  lv_obj_add_event_cb(_tempUp, onTempUp, LV_EVENT_CLICKED, this);

  createUnitLabel(parent, "\xC2\xB0", LV_ALIGN_CENTER, 90, -36);
  _bigState = createLabel(parent, "Set: --", LV_ALIGN_CENTER, 0, 28);
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
    snprintf(buf, sizeof(buf), "Set: %.0f", CDemandManager::getDegC());
  }
  lv_label_set_text(_bigState, buf);
}

void MainStatusScreen::onTempUp(lv_event_t*) {
  CDemandManager::deltaDemand(1);
}

void MainStatusScreen::onTempDown(lv_event_t*) {
  CDemandManager::deltaDemand(-1);
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
  _primeStatus = createLabel(parent, "Standby", LV_ALIGN_CENTER, 0, -30);

  _primeBtn = lv_button_create(parent);
  lv_obj_set_pos(_primeBtn, 60, 10);
  lv_obj_set_size(_primeBtn, 100, 30);
  lv_obj_set_style_bg_color(_primeBtn, lv_color_make(0xFF,0x8C,0x00), 0);
  lv_obj_set_style_border_width(_primeBtn, 0, 0);
  lv_obj_t* plbl = lv_label_create(_primeBtn);
  lv_label_set_text(plbl, "PRIME");
  lv_obj_center(plbl);
  lv_obj_add_event_cb(_primeBtn, onPrimeBtn, LV_EVENT_CLICKED, this);

  _resetFuelBtn = lv_button_create(parent);
  lv_obj_set_pos(_resetFuelBtn, 170, 10);
  lv_obj_set_size(_resetFuelBtn, 100, 30);
  lv_obj_set_style_bg_color(_resetFuelBtn, lv_color_make(0x44,0x44,0x44), 0);
  lv_obj_set_style_border_width(_resetFuelBtn, 0, 0);
  lv_obj_t* rlbl = lv_label_create(_resetFuelBtn);
  lv_label_set_text(rlbl, "RESET");
  lv_obj_center(rlbl);
  lv_obj_add_event_cb(_resetFuelBtn, onResetFuel, LV_EVENT_CLICKED, this);

  createLabel(parent, "Fuel used:", LV_ALIGN_CENTER, 0, 60);
  _primeFuelUsed = createValueLabel(parent, "0.0 L", LV_ALIGN_CENTER, 0, 80);
}

void MainStatusScreen::updatePriming() {
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f L", FuelGauge.Used_mL() / 1000.0f);
  lv_label_set_text(_primeFuelUsed, buf);
}

void MainStatusScreen::onPrimeBtn(lv_event_t* e) {
  auto* self = static_cast<MainStatusScreen*>(lv_event_get_user_data(e));
  // Toggle prime state
  lv_obj_t* lbl = lv_obj_get_child(self->_primeBtn, 0);
  if (!lbl) return;
  const char* text = lv_label_get_text(lbl);
  if (strcmp(text, "PRIME") == 0) {
    reqPumpPrime(true);
    lv_label_set_text(lbl, "STOP");
    lv_label_set_text(self->_primeStatus, "Priming...");
    lv_obj_set_style_bg_color(self->_primeBtn, lv_color_make(0xCC,0x33,0x00), 0);
  } else {
    reqPumpPrime(false);
    lv_label_set_text(lbl, "PRIME");
    lv_label_set_text(self->_primeStatus, "Standby");
    lv_obj_set_style_bg_color(self->_primeBtn, lv_color_make(0xFF,0x8C,0x00), 0);
  }
}

void MainStatusScreen::onResetFuel(lv_event_t* e) {
  FuelGauge.reset();
}

void MainStatusScreen::onSettings() {
  auto* s = new SettingsMenuScreen();
  s->onLoad();
  lv_scr_load(s->getScreen());
}

// ── Timer callback ──────────────────────────────────────────
void MainStatusScreen::onTimer() {
  updateHeaderClock();
  updateTelemetry();
  updateBigTemp();
  updateClockTabContent();
  updatePriming();
}
