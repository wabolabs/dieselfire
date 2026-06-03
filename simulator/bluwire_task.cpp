#include "cfg/DFConfig.h"
#include "cfg/pins.h"
#include "../src/Protocol/Protocol.h"
#include "../src/Utility/UtilClasses.h"
#include "../src/Utility/DataFilter.h"
#include "../src/Utility/FuelGauge.h"
#include "../src/Utility/macros.h"
#include "../src/Protocol/BlueWireTask.h"
#include "vserial.h"

// BlueWire serial port — linked from main.cpp
extern VirtualSerial BlueWireSerial;

#define RX_DATA_TIMOUT 50

CommStates CommState;
CProtocol DefaultDFParams(CProtocol::CtrlMode);
CModeratedFrame OEMCtrlFrame;
CModeratedFrame HeaterFrame1;
CProtocol HeaterFrame2;
CProtocolPackage reportHeaterData;
CProtocolPackage primaryHeaterData;
char dbgMsg[BLUEWIRE_MSGQUEUESIZE];

static bool bHasOEMController = false;
static bool bHasOEMLCDController = false;
static bool bHasHtrData = false;

extern bool bReportRecyleEvents;
extern bool bReportOEMresync;
extern bool bReportBlueWireData;
extern sFilteredData FilteredSamples;

QueueHandle_t BlueWireMsgQueue = NULL;
QueueHandle_t BlueWireRxQueue = NULL;
QueueHandle_t BlueWireTxQueue = NULL;
SemaphoreHandle_t BlueWireSemaphore = NULL;

// Debug frame capture — read by BWDebugScreen
extern uint8_t g_lastTxFrame[24];
extern uint8_t g_lastRxFrame[24];
void captureTxFrame(const uint8_t* data);
void captureRxFrame(const uint8_t* data);

// Simulated TX manager — writes frame to VirtualSerial instead of using
// hardware timer + UART gate like the real CTxManage.
struct SimTxMgr {
  CProtocol m_TxFrame;
  unsigned long m_nStartTime = 0;

  void PrepareFrame(const CProtocol& Frame, bool isDFmaster) {
    m_TxFrame = Frame;
    m_TxFrame.setCRC();
  }
  void Start(unsigned long timenow) {
    // Original code: starts HW timer for Tx gate.
    // Actual serial write happens in CheckTx when timer fires.
    m_nStartTime = timenow;
  }
  bool CheckTx(unsigned long timenow) {
    // After ~10ms, write the frame and complete, like original's HW timer ISR.
    if (m_nStartTime != 0 && (timenow - m_nStartTime) >= 10) {
      BlueWireSerial.write(m_TxFrame.Data, 24);
      BlueWireSerial.flush();
      captureTxFrame(m_TxFrame.Data);
      m_nStartTime = 0;
      return true;
    }
    return false;
  }
  const CProtocol& getFrame() const { return m_TxFrame; }
  void begin() {}
  void setCallback(std::function<void(const char*)>) {}
  void reqPrime(bool on) {}
};
static SimTxMgr TxManage;

bool validateFrame(const CProtocol& frame, const char* name);
void DebugReportFrame(const char* hdr, const CProtocol& Frame, const char* ftr, char* msg);
void pushDebugMsg(const char* msg) {
  if (BlueWireMsgQueue)
    xQueueSend(BlueWireMsgQueue, msg, 0);
}

void BlueWireTask(void*) {
  static unsigned long lastRxTime = 0;
  static unsigned long moderator = 50;
  bool isDFmaster = false;

  BlueWireMsgQueue = xQueueCreate(4, BLUEWIRE_MSGQUEUESIZE);
  BlueWireRxQueue = xQueueCreate(4, BLUEWIRE_DATAQUEUESIZE);
  BlueWireTxQueue = xQueueCreate(4, BLUEWIRE_DATAQUEUESIZE);
  BlueWireSemaphore = xSemaphoreCreateBinary();

  TxManage.begin();

  DefaultDFParams.setHeaterDemand(23);
  DefaultDFParams.setTemperature_Actual(22);
  DefaultDFParams.setSystemVoltage(12.0);
  DefaultDFParams.setPump_Min(1.6f);
  DefaultDFParams.setPump_Max(5.5f);
  DefaultDFParams.setFan_Min(1680);
  DefaultDFParams.setFan_Max(4500);
  DefaultDFParams.Controller.FanSensor = 1;

  CommState.setCallback(pushDebugMsg);

  for (;;) {
    sRxData BlueWireRxData;
    unsigned long timenow = millis();
    unsigned long RxTimeElapsed = timenow - lastRxTime;

    if (BlueWireSerial.available()) {
      BlueWireRxData.setValue(BlueWireSerial.read());
      lastRxTime = timenow;
    }

    if (RxTimeElapsed > RX_DATA_TIMOUT) {
      if (CommState.is(CommStates::OEMCtrlRx) ||
          CommState.is(CommStates::HeaterRx1) ||
          CommState.is(CommStates::HeaterRx2)) {
        if (RxTimeElapsed >= moderator) {
          moderator += 10;
          if (bReportRecyleEvents) {
            sprintf(dbgMsg, "%ldms - ", RxTimeElapsed);
            pushDebugMsg(dbgMsg);
          }
          if (CommState.is(CommStates::OEMCtrlRx)) {
            bHasOEMController = false;
            bHasOEMLCDController = false;
            if (bReportRecyleEvents)
              pushDebugMsg("Timeout collecting OEM controller data, returning to Idle State\r\n");
          } else if (CommState.is(CommStates::HeaterRx1)) {
            bHasHtrData = false;
            if (bReportRecyleEvents)
              pushDebugMsg("Timeout collecting OEM heater response data, returning to Idle State\r\n");
          } else {
            bHasHtrData = false;
            if (bReportRecyleEvents)
              pushDebugMsg("Timeout collecting BTC heater response data, returning to Idle State\r\n");
          }
        }
        if (bReportRecyleEvents)
          pushDebugMsg("Recycling blue wire serial interface\r\n");
        CommState.set(CommStates::ExchangeComplete);
      }
    }

    switch (CommState.get()) {

      case CommStates::Idle:
        moderator = 50;

        if (RxTimeElapsed >= 940) {
          bHasHtrData = false;
          bHasOEMController = false;
          bHasOEMLCDController = false;
          isDFmaster = true;
          TxManage.PrepareFrame(DefaultDFParams, isDFmaster);
          TxManage.Start(timenow);
          CommState.set(CommStates::TxStart);
          break;
        }

        if (BlueWireRxData.available() && (RxTimeElapsed > (RX_DATA_TIMOUT + 10))) {
          if (bReportOEMresync) {
            sprintf(dbgMsg, "Re-sync'd with OEM Controller. %ldms Idle time.\r\n", RxTimeElapsed);
            pushDebugMsg(dbgMsg);
          }
          bHasHtrData = false;
          bHasOEMController = true;
          CommState.set(CommStates::OEMCtrlRx);
        } else {
          break;
        }

      case CommStates::OEMCtrlRx:
        if (BlueWireRxData.available()) {
          if (CommState.collectData(OEMCtrlFrame, BlueWireRxData.getValue()))
            CommState.set(CommStates::OEMCtrlValidate);
        }
        break;

      case CommStates::OEMCtrlValidate:
        if (!validateFrame(OEMCtrlFrame, "OEM"))
          break;
        OEMCtrlFrame.setTime();
        bHasOEMLCDController = (OEMCtrlFrame.Controller.Byte0 != 0x78);
        CommState.set(CommStates::HeaterRx1);
        break;

      case CommStates::HeaterRx1:
        if (BlueWireRxData.available()) {
          if (CommState.collectData(HeaterFrame1, BlueWireRxData.getValue()))
            CommState.set(CommStates::HeaterValidate1);
        }
        break;

      case CommStates::HeaterValidate1:
        if (!validateFrame(HeaterFrame1, "RX1")) {
          bHasHtrData = false;
          break;
        }
        bHasHtrData = true;
        HeaterFrame1.setTime();
        while (BlueWireSerial.available()) {
          pushDebugMsg("DUMPED ROGUE RX DATA\r\n");
          BlueWireSerial.read();
        }
        BlueWireSerial.flush();
        primaryHeaterData.set(HeaterFrame1, OEMCtrlFrame);
        if (bReportBlueWireData)
          primaryHeaterData.reportFrames(true, pushDebugMsg);
        isDFmaster = false;
        TxManage.PrepareFrame(OEMCtrlFrame, isDFmaster);
        CommState.set(CommStates::TxStart);
        break;

      case CommStates::TxStart:
        xQueueSend(BlueWireTxQueue, TxManage.getFrame().Data, 0);
        TxManage.Start(timenow);
        CommState.set(CommStates::TxInterval);
        break;

      case CommStates::TxInterval:
        lastRxTime = timenow;
        if (TxManage.CheckTx(timenow))
          CommState.set(CommStates::HeaterRx2);
        break;

      case CommStates::HeaterRx2:
        if (BlueWireRxData.available()) {
          if (CommState.collectData(HeaterFrame2, BlueWireRxData.getValue()))
            CommState.set(CommStates::HeaterValidate2);
        }
        break;

      case CommStates::HeaterValidate2:
        if (!validateFrame(HeaterFrame2, "RX2")) {
          bHasHtrData = false;
          break;
        }
        bHasHtrData = true;
        captureRxFrame(HeaterFrame2.Data);
        xQueueSend(BlueWireRxQueue, HeaterFrame2.Data, 0);
        if (!bHasOEMController)
          primaryHeaterData.set(HeaterFrame2, TxManage.getFrame());
        if (bReportBlueWireData) {
          reportHeaterData.set(HeaterFrame2, TxManage.getFrame());
          reportHeaterData.reportFrames(false, pushDebugMsg);
        }
        CommState.set(CommStates::ExchangeComplete);
        break;

      case CommStates::ExchangeComplete:
        xSemaphoreGive(BlueWireSemaphore);
        CommState.set(CommStates::Idle);
        break;
    }

    if (!BlueWireSerial.available())
      vTaskDelay(1);
  }
}

bool validateFrame(const CProtocol& frame, const char* name) {
  if (!frame.verifyCRC(pushDebugMsg)) {
    sprintf(dbgMsg, "\007Bad CRC detected for %s frame - restarting blue wire's serial port\r\n", name);
    pushDebugMsg(dbgMsg);
    dbgMsg[0] = 0;
    DebugReportFrame("BAD CRC:", frame, "\r\n", dbgMsg);
    pushDebugMsg(dbgMsg);
    CommState.set(CommStates::ExchangeComplete);
    return false;
  }
  return true;
}

void DebugReportFrame(const char* hdr, const CProtocol& Frame, const char* ftr, char* msg) {
  strcat(msg, hdr);
  for (int i = 0; i < 24; i++) {
    char str[8];
    sprintf(str, " %02X", Frame.Data[i]);
    strcat(msg, str);
  }
  strcat(msg, ftr);
}

bool hasOEMcontroller() { return bHasOEMController; }
bool hasOEMLCDcontroller() { return bHasOEMLCDController; }
bool hasHtrData() { return bHasHtrData; }

int getBlueWireStat() {
  int stat = 0;
  if (!bHasHtrData) stat |= 0x01;
  if (bHasOEMController) stat |= 0x02;
  return stat;
}

const char* getBlueWireStatStr() {
  static const char* BlueWireStates[] = {"BTC,Htr", "BTC", "OEM,Htr", "OEM"};
  return BlueWireStates[getBlueWireStat()];
}

void reqPumpPrime(bool on) { TxManage.reqPrime(on); }

// Debug frame storage
uint8_t g_lastTxFrame[24] = {};
uint8_t g_lastRxFrame[24] = {};

void captureTxFrame(const uint8_t* data) { memcpy(g_lastTxFrame, data, 24); }
void captureRxFrame(const uint8_t* data) { memcpy(g_lastRxFrame, data, 24); }
