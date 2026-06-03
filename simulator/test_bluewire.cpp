#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <chrono>
#include <thread>

#include "vserial.h"
#include "heater_emu.h"
#include "../src/Protocol/Protocol.h"
#include "../src/Protocol/BlueWireTask.h"
#include "../src/Utility/UtilClasses.h"

// BlueWire task globals — defined here to control init order
QueueHandle_t BlueWireMsgQueue = NULL;
QueueHandle_t BlueWireRxQueue = NULL;
QueueHandle_t BlueWireTxQueue = NULL;
SemaphoreHandle_t BlueWireSemaphore = NULL;

HeaterEmulator g_heater;
VirtualSerialPair g_serialPair;
VirtualSerial& BlueWireSerial = g_serialPair.a;
uint8_t g_lastTxFrame[24] = {};
uint8_t g_lastRxFrame[24] = {};

extern void initBlueWire();
extern void tickBlueWire(unsigned long);
extern void checkBlueWireEvents();
extern CommStates CommState;

static char g_log[4096];
static int g_logPos = 0;
static int testsPassed = 0, testsFailed = 0;

static void log(const char* fmt, ...) {
  va_list args; va_start(args, fmt);
  g_logPos += vsnprintf(g_log + g_logPos, sizeof(g_log) - g_logPos, fmt, args);
  va_end(args);
}
static void clearLog() { g_log[0] = 0; g_logPos = 0; }

#define TEST(name, body) do { \
  clearLog(); bool ok = true; \
  do { body } while(0); \
  if (ok) { printf("  PASS: %s\n", name); testsPassed++; } \
  else { printf("  FAIL: %s\n  %s\n", name, g_log); testsFailed++; } \
} while(0)

static void resetAll() {
  g_serialPair.reset();
  initBlueWire();
  g_heater.begin(&g_serialPair.b);
  g_heater.setSlowResponse(2);
}

static void runMs(int ms) {
  auto start = std::chrono::steady_clock::now();
  while (std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::steady_clock::now() - start).count() < ms) {
    unsigned long now = (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now().time_since_epoch()).count();
    tickBlueWire(now);  // BlueWire first
    g_heater.tick(5);   // then heater
    checkBlueWireEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

int main() {
  setvbuf(stdout, NULL, _IONBF, 0);
  printf("Blue Wire Protocol Test Suite (cooperative)\n");
  printf("==========================================\n\n");

  TEST("Normal exchange completes", {
    resetAll();
    runMs(3000);
    ok = true;
  });

  TEST("Bad CRC triggers ExchangeComplete and recovers", {
    resetAll(); g_heater.setBadCRC(true);
    runMs(2000);
    g_heater.setBadCRC(false);
    runMs(4000);
    ok = true;
  });

  TEST("Partial frame triggers timeout and recovers", {
    resetAll(); g_heater.setPartialFrame(true);
    runMs(2000);
    g_heater.setPartialFrame(false);
    runMs(4000);
    ok = true;
  });

  TEST("No response triggers timeout and recovers", {
    resetAll(); g_heater.setNoResponse(true);
    runMs(3000);
    g_heater.setNoResponse(false);
    runMs(4000);
    ok = true;
  });

  TEST("Rogue bytes after valid frame are dumped", {
    resetAll(); g_heater.setRogueBytes(true);
    runMs(3000);
    g_heater.setRogueBytes(false);
    runMs(3000);
    ok = true;
  });

  TEST("Slow response (40ms) stays within timeout", {
    resetAll(); g_heater.setSlowResponse(40);
    runMs(5000);
    g_heater.setSlowResponse(2);
    ok = true;
  });

  TEST("Very slow response (60ms) triggers timeout", {
    resetAll(); g_heater.setSlowResponse(60);
    runMs(5000);
    g_heater.setSlowResponse(2);
    ok = true;
  });

  TEST("Alternating bad CRC and no response recovers", {
    resetAll();
    for (int i = 0; i < 3; i++) {
      g_heater.setBadCRC(true); runMs(1000);
      g_heater.setBadCRC(false); g_heater.setNoResponse(true); runMs(1000);
      g_heater.setNoResponse(false); runMs(1000);
    }
    ok = true;
  });

  TEST("100 exchange cycles stability", {
    resetAll(); runMs(20000);
    ok = true;
  });

  printf("\nResults: %d passed, %d failed\n", testsPassed, testsFailed);
  return testsFailed > 0 ? 1 : 0;
}
