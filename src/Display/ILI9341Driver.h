#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

class ILI9341Driver : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_GT911 _touch_instance;

public:
  ILI9341Driver();
};
