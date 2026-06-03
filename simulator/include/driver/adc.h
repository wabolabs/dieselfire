// Mock: driver/adc.h for PC simulator
#pragma once

#include <stdint.h>

typedef uint32_t adc_unit_t;
typedef int adc_channel_t;
typedef int adc_atten_t;
typedef int adc_bits_width_t;

typedef enum {
    ADC2_CHANNEL_0 = 0,
    ADC2_CHANNEL_1 = 1,
    ADC2_CHANNEL_2 = 2,
    ADC2_CHANNEL_3 = 3,
    ADC2_CHANNEL_4 = 4,
    ADC2_CHANNEL_5 = 5,
    ADC2_CHANNEL_6 = 6,
    ADC2_CHANNEL_7 = 7,
    ADC2_CHANNEL_8 = 8,
    ADC2_CHANNEL_9 = 9
} adc2_channel_t;

static inline int adc_gpio_init(adc_unit_t, adc_channel_t) { return 0; }
static inline int adc1_config_channel_atten(adc_channel_t, adc_atten_t) { return 0; }
static inline int adc1_get_raw(adc_channel_t) { return 0; }
