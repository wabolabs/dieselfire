#include "mock_data.h"

SimState g_state;

void updateSimulation(int ms) {
  static int accum = 0;
  accum += ms;
  if (accum < 50) return;
  accum = 0;

  // State machine advancing through heater cycle
  // Full cycle takes ~30 seconds
  static unsigned tick = 0;
  tick++;

  // Advance run state
  if (tick == 50)   g_state.runState = 1;  // Starting
  if (tick == 100)  g_state.runState = 2;  // GlowOn
  if (tick == 200)  g_state.runState = 3;  // Ignited
  if (tick == 250)  g_state.runState = 4;  // Running
  if (tick == 500)  g_state.runState = 5;  // Shutdown
  if (tick == 550) { g_state.runState = 0; tick = 0; g_state.fuelUsed_liters = 0; }

  // Update telemetry based on state
  switch (g_state.runState) {
    case 0: // Off / Standby
      g_state.ambientTemp = 10.0f + (tick % 20) * 0.1f;
      g_state.heatExchangerTemp = g_state.ambientTemp;
      g_state.pumpHz = 0;
      g_state.fanRPM = 0;
      g_state.glowPower = 0;
      g_state.batteryVolts = 12.6f;
      break;
    case 1: // Starting
      g_state.pumpHz = 2.5f;
      g_state.fanRPM = 800;
      g_state.glowPower = 35.0f;
      g_state.batteryVolts = 12.3f;
      break;
    case 2: // GlowOn
      g_state.pumpHz = 3.5f;
      g_state.fanRPM = 1200;
      g_state.glowPower = 70.0f;
      g_state.batteryVolts = 12.1f;
      g_state.heatExchangerTemp += 0.3f;
      break;
    case 3: // Ignited
      g_state.pumpHz = 4.2f;
      g_state.fanRPM = 1800;
      g_state.glowPower = 45.0f;
      g_state.batteryVolts = 12.2f;
      g_state.heatExchangerTemp += 1.5f;
      break;
    case 4: // Running
      g_state.pumpHz = 3.8f + (tick % 10) * 0.1f;
      g_state.fanRPM = 2200 + (tick % 5) * 50;
      g_state.glowPower = 5.0f;
      g_state.batteryVolts = 12.8f;
      g_state.ambientTemp += 0.02f;
      g_state.heatExchangerTemp = 65.0f + (tick % 20) * 0.5f;
      g_state.fuelUsed_liters += 0.001f;
      break;
    case 5: // Shutdown
      g_state.pumpHz = 1.5f;
      g_state.fanRPM = 3000;
      g_state.glowPower = 0;
      g_state.heatExchangerTemp -= 2.0f;
      g_state.batteryVolts = 12.5f;
      break;
  }
}
