#include <stdint.h>
#include <Arduino.h>
#include "DFConfig.h"

// ── DieselFire PCB Pinout ────────────────────────────────────
// ESP32-S3-WROOM-1-N8R8 on 100×100mm 2-layer board

// === Native USB (USB-C, programming + debug) ===
// USB D+ (GPIO19), USB D- (GPIO20) handled by hardware USB CDC

// === Display: ILI9341 SPI (320×240 TFT + FPC) ===
static const uint8_t TFT_MOSI = 18;
static const uint8_t TFT_SCK  = 17;
static const uint8_t TFT_MISO = 16;
static const uint8_t TFT_CS   = 15;
static const uint8_t TFT_DC   = 14;
static const uint8_t TFT_RST  = 13;
static const uint8_t TFT_BL   = 12;

// === Touch: GT911 I2C (capacitive touch) ===
static const uint8_t TOUCH_SDA = 21;
static const uint8_t TOUCH_SCL = 10;
static const uint8_t TOUCH_INT = 9;
static const uint8_t TOUCH_RST = 8;

// === Sensor I2C Bus (BME280 + DS3231 + GT911 share bus) ===
static const uint8_t I2C_SDA = 21;
static const uint8_t I2C_SCL = 22;

// === Blue Wire (heater half-duplex UART) ===
static const uint8_t BLUE_TX      = 43;
static const uint8_t BLUE_RX      = 44;
static const uint8_t BLUE_TX_GATE = 47;

// === Sensors ===
static const uint8_t DS18B20_Pin = 45;
static const uint8_t MQ7_AOUT    = 4;   // ADC1_CHANNEL_3 on ESP32-S3
static const uint8_t MQ7_DOUT    = 5;   // Digital threshold output

// === LEDs ===
static const uint8_t LED_STATUS   = 48;  // Green, active-low
static const uint8_t LED_CO_ALARM = 44;  // Red, active-low

// === 433MHz UHF Remote ===
static const uint8_t Rx433MHz_pin = 41;

// === Buttons ===
static const uint8_t BTN_POWER = 0;

// Legacy aliases for code compatibility with existing source
#define GPIOout1_pin  LED_STATUS
#define GPIOout2_pin  LED_CO_ALARM
#define LED_Pin       LED_STATUS
#define Rx1Pin        BLUE_RX
#define Tx1Pin        BLUE_TX
#define TxEnbPin      BLUE_TX_GATE
