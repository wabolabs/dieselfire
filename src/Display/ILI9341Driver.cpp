#include "ILI9341Driver.h"
#include "../cfg/pins.h"
#include "../cfg/DFConfig.h"

ILI9341Driver::ILI9341Driver() {
  auto bus_cfg = _bus_instance.config();
  bus_cfg.spi_host = TFT_SPI_HOST;
  bus_cfg.spi_mode = 0;
  bus_cfg.freq_write = TFT_SPI_CLOCK_HZ;
  bus_cfg.freq_read = TFT_SPI_CLOCK_HZ / 2;
  bus_cfg.pin_sclk = TFT_SCK;
  bus_cfg.pin_mosi = TFT_MOSI;
  bus_cfg.pin_miso = TFT_MISO;
  bus_cfg.pin_dc = TFT_DC;
  _bus_instance.config(bus_cfg);

  auto panel_cfg = _panel_instance.config();
  panel_cfg.pin_cs = TFT_CS;
  panel_cfg.pin_rst = TFT_RST;
  panel_cfg.pin_busy = -1;
  panel_cfg.panel_width = TFT_WIDTH;
  panel_cfg.panel_height = TFT_HEIGHT;
  panel_cfg.memory_width = TFT_WIDTH;
  panel_cfg.memory_height = TFT_HEIGHT;
  panel_cfg.offset_x = 0;
  panel_cfg.offset_y = 0;
  panel_cfg.readable = false;
  panel_cfg.invert = false;
  panel_cfg.rgb_order = false;
  panel_cfg.dlen_16bit = false;
  panel_cfg.bus_shared = true;
  _panel_instance.config(panel_cfg);

  auto light_cfg = _light_instance.config();
  light_cfg.pin_bl = TFT_BL;
  light_cfg.invert = false;
  light_cfg.freq = 44100;
  light_cfg.pwm_channel = 0;
  _light_instance.config(light_cfg);

  auto touch_cfg = _touch_instance.config();
  touch_cfg.i2c_addr = 0x5D;
  touch_cfg.i2c_port = 1;
  touch_cfg.pin_sda = TOUCH_SDA;
  touch_cfg.pin_scl = TOUCH_SCL;
  touch_cfg.pin_int = TOUCH_INT;
  touch_cfg.pin_rst = TOUCH_RST;
  touch_cfg.freq = I2C_BUS_SPEED;
  touch_cfg.x_min = 0;
  touch_cfg.x_max = TFT_WIDTH - 1;
  touch_cfg.y_min = 0;
  touch_cfg.y_max = TFT_HEIGHT - 1;
  _touch_instance.config(touch_cfg);

  _panel_instance.setBus(&_bus_instance);
  _panel_instance.setLight(&_light_instance);
  _panel_instance.setTouch(&_touch_instance);

  setPanel(&_panel_instance);
}
