#include "mock/mock_data.h"
#include "Display/Screens/DieselScreen.h"
#include "Display/Screens/MainStatusScreen.h"
#include "Utility/DebugPort.h"
#include "cfg/DFConfig.h"

#include <SDL2/SDL.h>
#include <lvgl.h>

// SDL display driver for LVGL
static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* texture = nullptr;
static lv_display_t* display = nullptr;
static uint32_t* sdl_buf = nullptr;
static void sdl_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  int32_t w = lv_area_get_width(area);
  int32_t h = lv_area_get_height(area);
  uint32_t* src = (uint32_t*)px_map;

  // Convert LVGL ARGB8888 to SDL pixels
  SDL_Rect r = { area->x1, area->y1, w, h };
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint32_t c = src[y * w + x];
      uint8_t a = (c >> 24) & 0xFF;
      uint8_t r8 = (c >> 16) & 0xFF;
      uint8_t g8 = (c >> 8) & 0xFF;
      uint8_t b8 = c & 0xFF;
      sdl_buf[(area->y1 + y) * TFT_WIDTH + (area->x1 + x)] =
        (a << 24) | (r8 << 16) | (g8 << 8) | b8;
    }
  }
  SDL_UpdateTexture(texture, &r, sdl_buf + area->y1 * TFT_WIDTH + area->x1, TFT_WIDTH * 4);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
  lv_display_flush_ready(disp);
}

static void sdl_mouse_read(lv_indev_t* indev, lv_indev_data_t* data) {
  int x, y;
  uint32_t buttons = SDL_GetMouseState(&x, &y);
  data->point.x = x;
  data->point.y = y;
  data->state = (buttons & SDL_BUTTON_LMASK) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

int main(int, char**) {
  // Init SDL
  SDL_Init(SDL_INIT_VIDEO);
  window = SDL_CreateWindow("DieselFire Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            TFT_WIDTH, TFT_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                              TFT_WIDTH, TFT_HEIGHT);
  sdl_buf = new uint32_t[TFT_WIDTH * TFT_HEIGHT]();

  // Init LVGL
  lv_init();
  DieselScreen::initTheme();

  size_t bufSize = TFT_WIDTH * TFT_HEIGHT * 4;
  uint8_t* buf1 = new uint8_t[bufSize];
  uint8_t* buf2 = new uint8_t[bufSize];

  display = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
  lv_display_set_flush_cb(display, sdl_flush_cb);
  lv_display_set_buffers(display, buf1, buf2, bufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, sdl_mouse_read);

  // Load main screen
  auto* mainScreen = new MainStatusScreen();
  mainScreen->onLoad();
  lv_scr_load(mainScreen->getScreen());

  // Main loop
  uint32_t lastTick = SDL_GetTicks();
  bool running = true;

  while (running) {
    uint32_t now = SDL_GetTicks();
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
    }

    // Update simulation
    updateSimulation(now - lastTick);

    // Update clock
    if (now - lastTick >= 5) {
      lv_tick_inc(now - lastTick);
      lastTick = now;
    }

    lv_task_handler();
    SDL_Delay(5);
  }

  delete[] sdl_buf;
  delete[] buf1;
  delete[] buf2;
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
