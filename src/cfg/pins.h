#include <stdint.h>
#include <driver/adc.h>
#include "DFConfig.h"

// ── DieselFire PCB Pinout ────────────────────────────────────
// ESP32-S3-WROOM-1-N8R8 on 100×100mm 2-layer board

// === Native USB (USB-C, programming + debug) ===
// USB D+ (IO19), USB D- (IO20) — handled by hardware, no GPIO config needed
// Debug serial uses internal USB CDC, not dedicated UART pins.

// === Display: ILI9341 SPI (320×240 TFT + FPC) ===
const gpio_num_t TFT_MOSI = GPIO_NUM_18;
const gpio_num_t TFT_SCK  = GPIO_NUM_17;
const gpio_num_t TFT_MISO = GPIO_NUM_16;
const gpio_num_t TFT_CS   = GPIO_NUM_15;
const gpio_num_t TFT_DC   = GPIO_NUM_14;
const gpio_num_t TFT_RST  = GPIO_NUM_13;
const gpio_num_t TFT_BL   = GPIO_NUM_12;

// === Touch: GT911 I2C (capacitive touch) ===
const gpio_num_t TOUCH_SDA = GPIO_NUM_21;
const gpio_num_t TOUCH_SCL = GPIO_NUM_10;
const gpio_num_t TOUCH_INT = GPIO_NUM_9;
const gpio_num_t TOUCH_RST = GPIO_NUM_8;

// === Sensor I2C Bus (BME280 + DS3231 + GT911 share bus) ===
const gpio_num_t I2C_SDA = GPIO_NUM_21;
const gpio_num_t I2C_SCL = GPIO_NUM_22;

// === Blue Wire (heater half-duplex UART) ===
const gpio_num_t BLUE_TX     = GPIO_NUM_43;
const gpio_num_t BLUE_RX     = GPIO_NUM_44;
const gpio_num_t BLUE_TX_GATE = GPIO_NUM_47;  // 74LCX125 Tx enable

// === Sensors ===
const gpio_num_t DS18B20_Pin = GPIO_NUM_45;   // OneWire external temp
const adc1_channel_t MQ7_ADC = ADC1_CHANNEL_2; // GPIO4, ADC1
const gpio_num_t MQ7_DOUT    = GPIO_NUM_5;    // CO digital alert

// === LEDs ===
const gpio_num_t LED_STATUS = GPIO_NUM_48;  // Green, active-low
const gpio_num_t LED_CO_ALARM = GPIO_NUM_44; // Red, active-low

// === 433MHz UHF Remote Receiver ===
const gpio_num_t Rx433MHz_pin = GPIO_NUM_41;  // on H2 expansion header

// === Buttons ===
const gpio_num_t BTN_POWER = GPIO_NUM_0;   // With pull-up
// BOOT/RESET button is on the module itself, no dedicated GPIO

// === Expansion Headers ===
// H1 (2×10): IO33,34,35,36,37,38,39,40, 3V3, GND, GND
// H2 (2×10): IO41,42,43,44,45,46,47,48, 3V3, GND, GND

// Legacy aliases for code compatibility with existing source
#define GPIOout1_pin  LED_STATUS
#define GPIOout2_pin  LED_CO_ALARM
#define LED_Pin       LED_STATUS
