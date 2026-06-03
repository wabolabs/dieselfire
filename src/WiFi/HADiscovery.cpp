#include "../cfg/DFConfig.h"

#if USE_MQTT == 1

#include "HADiscovery.h"
#include "../../lib/async-mqtt-client/src/AsyncMqttClient.h"
#include "../Utility/DebugPort.h"
#include "../Utility/NVStorage.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#include "DFMQTT.h"

extern AsyncMqttClient MQTTclient;

static const char* getMacSuffix() {
  static char suffix[8] = "";
  if (suffix[0] == 0) {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(suffix, sizeof(suffix), "%02X%02X%02X", mac[3], mac[4], mac[5]);
  }
  return suffix;
}

// Build a topic like: {prefix}/climate/{suffix}
static void topicCat(char* buf, size_t len, const char* suffix) {
  snprintf(buf, len, "%s/%s", getTopicPrefix(), suffix);
}

// Build discovery topic: homeassistant/{component}/dieselfire_{mac}_{entity}/config
static void discoveryTopic(char* buf, size_t len, const char* component, const char* entity) {
  snprintf(buf, len, "homeassistant/%s/dieselfire_%s_%s/config",
           component, getMacSuffix(), entity);
}

// Add the common device block to a discovery JSON object
static void addDeviceBlock(JsonObject& root) {
  JsonObject& dev = root.createNestedObject("device");
  JsonArray& ids = dev.createNestedArray("identifiers");
  ids.add(getMacSuffix());
  dev["name"] = "DieselFire Heater";
  dev["manufacturer"] = "DieselFire";
  dev["model"] = "DieselFire S3";
  dev["sw_version"] = "DieselFire";
}

void publishHAClimateDiscovery() {
  if (!MQTTclient.connected()) return;

  StaticJsonBuffer<1024> buf;
  JsonObject& root = buf.createObject();
  root["name"] = "DieselFire Heater";
  root["unique_id"] = "climate";
  root["object_id"] = "dieselfire_heater";
  addDeviceBlock(root);

  char t[128];
  topicCat(t, sizeof(t), "climate/mode/set");
  root["mode_command_topic"] = t;
  topicCat(t, sizeof(t), "climate/mode");
  root["mode_state_topic"] = t;
  topicCat(t, sizeof(t), "climate/temperature/set");
  root["temperature_command_topic"] = t;
  topicCat(t, sizeof(t), "climate/temperature");
  root["temperature_state_topic"] = t;
  topicCat(t, sizeof(t), "climate/current_temperature");
  root["current_temperature_topic"] = t;
  topicCat(t, sizeof(t), "climate/action");
  root["action_topic"] = t;
  topicCat(t, sizeof(t), "climate/preset/set");
  root["preset_mode_command_topic"] = t;
  topicCat(t, sizeof(t), "climate/preset");
  root["preset_mode_state_topic"] = t;

  JsonArray& modes = root.createNestedArray("modes");
  modes.add("off");
  modes.add("heat");
  modes.add("auto");

  JsonArray& presets = root.createNestedArray("preset_modes");
  presets.add("eco");
  presets.add("none");
  presets.add("boost");

  root["temperature_unit"] = "C";
  root["min_temp"] = 8;
  root["max_temp"] = 35;
  root["temp_step"] = 1;

  char discovery[128];
  discoveryTopic(discovery, sizeof(discovery), "climate", "climate");
  char payload[1536];
  root.printTo(payload);
  MQTTclient.publish(discovery, NVstore.getMQTTinfo().qos, true, payload);
  DebugPort.printf("HA: published climate discovery (%s)\r\n", discovery);
}

static const struct {
  const char* name;
  const char* entity;
  const char* stsTopic;
  const char* deviceClass;
  const char* unit;
} HA_SENSORS[] = {
  {"Heat Exchanger Temp", "heat_exchanger_temp", "TempBody", "temperature", "°C"},
  {"Run State",           "run_state",           "RunString", NULL,         NULL},
  {"Error State",         "error_state",         "ErrorString","problem",  NULL},
  {"Fuel Usage",          "fuel_usage",          "FuelUsage",  NULL,       "mL"},
  {"Fuel Rate",           "fuel_rate",           "FuelRate",   NULL,       "mL/h"},
  {"Fan Speed",           "fan_speed",           "FanRPM",     NULL,       "RPM"},
  {"Supply Voltage",      "supply_voltage",      "InputVoltage","voltage", "V"},
  {"Pump Frequency",      "pump_frequency",      "PumpActual", NULL,       "Hz"},
};

static const char* HA_STATE_TOPICS[] = {
  "Run",
  "ErrorState",
};

void publishHASensorsDiscovery() {
  if (!MQTTclient.connected()) return;

  char t[128];
  char discovery[128];
  char payload[1024];

  for (unsigned i = 0; i < sizeof(HA_SENSORS) / sizeof(HA_SENSORS[0]); i++) {
    StaticJsonBuffer<600> buf;
    JsonObject& root = buf.createObject();
    root["name"] = HA_SENSORS[i].name;
    root["unique_id"] = HA_SENSORS[i].entity;
    addDeviceBlock(root);
    topicCat(t, sizeof(t), HA_SENSORS[i].stsTopic);
    root["state_topic"] = t;
    if (HA_SENSORS[i].deviceClass) {
      root["device_class"] = HA_SENSORS[i].deviceClass;
    }
    if (HA_SENSORS[i].unit) {
      root["unit_of_measurement"] = HA_SENSORS[i].unit;
    }
    discoveryTopic(discovery, sizeof(discovery), "sensor", HA_SENSORS[i].entity);
    root.printTo(payload);
    MQTTclient.publish(discovery, NVstore.getMQTTinfo().qos, true, payload);
    DebugPort.printf("HA: published sensor discovery %s\r\n", discovery);
  }

  // Binary sensors
  for (unsigned i = 0; i < sizeof(HA_STATE_TOPICS) / sizeof(HA_STATE_TOPICS[0]); i++) {
    StaticJsonBuffer<400> buf;
    JsonObject& root = buf.createObject();
    root["name"] = HA_STATE_TOPICS[i];
    root["unique_id"] = HA_STATE_TOPICS[i];
    addDeviceBlock(root);
    topicCat(t, sizeof(t), HA_STATE_TOPICS[i]);
    root["state_topic"] = t;
    const char* binEntity = (i == 0) ? "running" : "has_error";
    root["payload_on"] = (i == 0) ? "1" : "1";
    root["payload_off"] = (i == 0) ? "0" : "0";
    discoveryTopic(discovery, sizeof(discovery), "binary_sensor", binEntity);
    root.printTo(payload);
    MQTTclient.publish(discovery, NVstore.getMQTTinfo().qos, true, payload);
    DebugPort.printf("HA: published binary_sensor discovery %s\r\n", discovery);
  }
}

void publishHADiscovery() {
  if (!MQTTclient.connected()) return;
  if (!NVstore.getMQTTinfo().haDiscovery) {
    DebugPort.println("HA: discovery disabled (haDiscovery=0)");
    return;
  }
  publishHAClimateDiscovery();
  publishHASensorsDiscovery();
}

#endif
