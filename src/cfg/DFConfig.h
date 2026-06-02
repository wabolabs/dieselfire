// Place Holder Config File - User config vars and defines to be moved here

// ── Hardware selection ───────────────────────────────────────
#define USE_ILI9341_DISPLAY  1    // ILI9341 320×240 TFT via SPI
#define USE_GT911_TOUCH      1    // GT911 capacitive touch via I2C
#define USE_BME280           1    // BME280 temp/humidity/pressure
#define USE_MQ7              1    // MQ-7 CO sensor
#define USE_LVGL             1    // LVGL graphics library

// ── Bluetooth options ───────────────────────────────────────
// ESP32-S3 native BLE is recommended; HC-05 not present on new PCB
#define USE_HC05_BLUETOOTH     0
#define USE_BLE_BLUETOOTH      1
#define USE_CLASSIC_BLUETOOTH  0

// ── WiFi options ────────────────────────────────────────────
#define USE_WIFI      1
#define USE_AP_ALWAYS 0
#define USE_OTA       1
#define USE_WEBSERVER 1
#define USE_MQTT      1
#define USE_HTTPS     0
#define USE_PORTAL_TRIGGER_PIN 0

// ── Debug reporting ─────────────────────────────────────────
#define REPORT_RAW_DATA               0
#define TERMINATE_OEM_LINE            0
#define TERMINATE_DF_LINE             0
#define REPORT_OEM_RESYNC             0
#define REPORT_STATE_MACHINE_TRANSITIONS 0
#define REPORT_BLUEWIRE_RECYCLES      1

// ── LED monitoring ──────────────────────────────────────────
#define RX_LED  1
#define BT_LED  0

// ── DS18B20 temperature sensing ─────────────────────────────
#define MIN_TEMPERATURE_INTERVAL 750

// ── Real Time Clock ─────────────────────────────────────────
#define RTC_USE_DS3231  1
#define RTC_USE_DS1307  0
#define RTC_USE_PCF8523 0

// ── Blue wire handling ────────────────────────────────────
#define SUPPORT_OEM_CONTROLLER 1

// ── Communications reporting ────────────────────────────────
#define REPORT_JSON_TRANSMIT 1

// ── Watchdog ────────────────────────────────────────────────
#define USE_SW_WATCHDOG 1

#define USE_SSL_LOOP_TASK 1

// FreeRTOS task priorities
#define TASK_PRIORITY_ARDUINO      3
#define TASK_PRIORITY_HEATERCOMMS  4
#define TASK_PRIORITY_SSL_CERT     1
#define TASK_PRIORITY_SSL_LOOP     1

// Display hardware parameters
#define TFT_SPI_HOST       SPI2_HOST  // HSPI
#define TFT_SPI_CLOCK_HZ   40000000   // 40 MHz
#define TFT_WIDTH          320
#define TFT_HEIGHT         240

#define I2C_BUS_SPEED      400000     // 400 kHz
