#include "BWDebugScreen.h"
#include "../heater_emu.h"
#include "Protocol/BlueWireTask.h"
#include <cstdio>
#include <cstring>

extern HeaterEmulator g_heater;
extern uint8_t g_lastTxFrame[24];
extern uint8_t g_lastRxFrame[24];

static const char* stateName(int s) {
  switch (s) {
    case 0: return "Idle";
    case 1: return "OEMCtrlRx";
    case 2: return "OEMCtrlValidate";
    case 3: return "HeaterRx1";
    case 4: return "HeaterValidate1";
    case 5: return "TxStart";
    case 6: return "TxInterval";
    case 7: return "HeaterRx2";
    case 8: return "HeaterValidate2";
    case 9: return "ExchangeComplete";
    default: return "?";
  }
}

static const char* heaterStateName(int rs) {
  switch (rs) {
    case 0: return "Off";
    case 1: return "Starting";
    case 2: return "Glowing";
    case 3: return "RetryPause";
    case 4: return "Ignited";
    case 5: return "Running";
    case 6: return "Stopping";
    case 7: return "Shutdown";
    case 8: return "Cooling";
    default: return "?";
  }
}

static void hexStr(const uint8_t* data, char* out) {
  for (int i = 0; i < 24; i++) {
    sprintf(out + i * 3, "%02X ", data[i]);
  }
  out[24 * 3] = 0;
}

BWDebugScreen::BWDebugScreen() : DieselScreen("BWDebug") {}

void BWDebugScreen::onLoad() {
  createHeader(_screen);
  createLabel(_screen, "Blue Wire Debug", LV_ALIGN_TOP_LEFT, 48, 3);

  lv_obj_t* cont = createContentArea();
  int y = 4;

  // Protocol state
  createLabel(cont, "Protocol", LV_ALIGN_TOP_LEFT, 4, y);
  _stateLabel = createValueLabel(cont, "Idle", LV_ALIGN_TOP_LEFT, 70, y);

  // Heater state
  y += 28;
  createLabel(cont, "Heater", LV_ALIGN_TOP_LEFT, 4, y);
  _heaterState = createValueLabel(cont, "Off", LV_ALIGN_TOP_LEFT, 70, y);

  y += 24;
  _heaterTemp = createValueLabel(cont, "Amb: --.-", LV_ALIGN_TOP_LEFT, 4, y);

  y += 18;
  _heaterPump = createValueLabel(cont, "Pump: --.- Hz", LV_ALIGN_TOP_LEFT, 4, y);

  y += 18;
  _heaterFan = createValueLabel(cont, "Fan: ---- RPM", LV_ALIGN_TOP_LEFT, 4, y);

  // TX frame hex
  y += 24;
  createLabel(cont, "TX", LV_ALIGN_TOP_LEFT, 4, y);
  _txHex = createValueLabel(cont, "(waiting)", LV_ALIGN_TOP_LEFT, 30, y);

  // RX frame hex
  y += 20;
  createLabel(cont, "RX", LV_ALIGN_TOP_LEFT, 4, y);
  _rxHex = createValueLabel(cont, "(waiting)", LV_ALIGN_TOP_LEFT, 30, y);

  // Fault injection toggles
  y += 28;
  createLabel(cont, "Faults", LV_ALIGN_TOP_LEFT, 4, y);
  y += 24;

  auto mkBtn = [&](const char* label, int x) {
    lv_obj_t* btn = lv_button_create(cont);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, 54, 26);
    lv_obj_set_style_bg_color(btn, lv_color_make(0x33,0x33,0x33), 0);
    lv_obj_set_style_bg_color(btn, lv_color_make(0xFF,0x8C,0x00), LV_STATE_CHECKED);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, label);
    lv_obj_center(lbl);
    lv_obj_add_event_cb(btn, onBtnClick, LV_EVENT_VALUE_CHANGED, this);
    return btn;
  };

  _btnBadCRC  = mkBtn("BadCRC",  4);
  _btnPartial = mkBtn("Partial", 62);
  _btnNoResp  = mkBtn("NoResp",  120);
  _btnRogue   = mkBtn("Rogue",   178);
  _btnPassive = mkBtn("Passive", 236);

  y += 30;
  createLabel(cont, "BW Debug", LV_ALIGN_TOP_LEFT, 4, y);
  _faultLabel = createValueLabel(cont, "active", LV_ALIGN_TOP_LEFT, 76, y);

  lv_obj_set_height(cont, y + 30);

  _timer = lv_timer_create([](lv_timer_t* t) {
    static_cast<BWDebugScreen*>(lv_timer_get_user_data(t))->onTimer();
  }, 300, this);
}

void BWDebugScreen::onTimer() {
  // Protocol state
  lv_label_set_text(_stateLabel, stateName(CommState.get()));

  // Heater state
  lv_label_set_text(_heaterState, heaterStateName(g_heater.getRunState()));

  char buf[64];
  snprintf(buf, sizeof(buf), "Amb: %.1f\xC2\xB0", g_heater.getAmbientTemp());
  lv_label_set_text(_heaterTemp, buf);
  snprintf(buf, sizeof(buf), "Pump: %.1f Hz", g_heater.getPumpHz());
  lv_label_set_text(_heaterPump, buf);
  snprintf(buf, sizeof(buf), "Fan: %d RPM", g_heater.getFanRPM());
  lv_label_set_text(_heaterFan, buf);

  // TX hex
  char hex[24 * 3 + 1];
  hexStr(g_lastTxFrame, hex);
  lv_label_set_text(_txHex, hex);

  // RX hex
  hexStr(g_lastRxFrame, hex);
  lv_label_set_text(_rxHex, hex);

  // Fault status
  const char* sep = "";
  buf[0] = 0;
  if (g_heater.getBadCRC())    { strcat(buf, "BadCRC "); }
  if (g_heater.getPartialFrame()) { strcat(buf, "Partial "); }
  if (g_heater.getNoResponse())   { strcat(buf, "NoResp "); }
  if (g_heater.getRogueBytes())   { strcat(buf, "Rogue "); }
  if (g_heater.getPassiveMode())  { strcat(buf, "Passive "); }
  if (buf[0] == 0) strcpy(buf, "none");
  lv_label_set_text(_faultLabel, buf);
}

void BWDebugScreen::onBtnClick(lv_event_t* e) {
  auto* target = lv_event_get_target(e);
  lv_obj_t* btn = (lv_obj_t*)target;
  bool checked = lv_obj_has_state(btn, LV_STATE_CHECKED);
  // Identify button by label text
  lv_obj_t* lbl = (lv_obj_t*)lv_obj_get_child(btn, 0);
  if (!lbl) return;
  const char* text = lv_label_get_text(lbl);

  if      (strcmp(text, "BadCRC")  == 0) g_heater.setBadCRC(checked);
  else if (strcmp(text, "Partial") == 0) g_heater.setPartialFrame(checked);
  else if (strcmp(text, "NoResp")  == 0) g_heater.setNoResponse(checked);
  else if (strcmp(text, "Rogue")   == 0) g_heater.setRogueBytes(checked);
  else if (strcmp(text, "Passive") == 0) g_heater.setPassiveMode(checked);
}
