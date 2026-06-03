#include "mock/mock_data.h"
#include "Display/Screens/DieselScreen.h"
#include "Display/Screens/MainStatusScreen.h"
#include "Utility/DebugPort.h"
#include "cfg/DFConfig.h"

#include <SDL2/SDL.h>
#include <lvgl.h>
#include <stdio.h>

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* texture = nullptr;
static lv_display_t* display = nullptr;

static void sdl_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  SDL_UpdateTexture(texture, nullptr, px_map, TFT_WIDTH * sizeof(lv_color32_t));
  SDL_RenderClear(renderer);
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
  if (SDL_Init(SDL_INIT_VIDEO) < 0) { printf("SDL_Init failed\n"); return 1; }
  window = SDL_CreateWindow("DieselFire", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            TFT_WIDTH, TFT_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) { printf("SDL_CreateWindow failed\n"); return 1; }
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                              TFT_WIDTH, TFT_HEIGHT);

  lv_init();

  size_t bufSize = TFT_WIDTH * TFT_HEIGHT * sizeof(lv_color32_t);
  uint8_t* fb = new uint8_t[bufSize]();

  display = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
  lv_display_set_flush_cb(display, sdl_flush_cb);
  lv_display_set_buffers(display, fb, nullptr, bufSize, LV_DISPLAY_RENDER_MODE_DIRECT);
  lv_display_set_default(display);
  DieselScreen::initTheme();

  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, sdl_mouse_read);

  auto* mainScreen = new MainStatusScreen();
  mainScreen->onLoad();
  lv_scr_load(mainScreen->getScreen());
  lv_refr_now(display);

  uint32_t lastTick = SDL_GetTicks();
  bool running = true;

  while (running) {
    uint32_t now = SDL_GetTicks();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
    }
    uint32_t delta = now - lastTick;
    lastTick = now;
    lv_tick_inc(delta);
    updateSimulation(delta);
    lv_task_handler();
    uint32_t elapsed = SDL_GetTicks() - now;
    if (elapsed < 16) SDL_Delay(16 - elapsed);
  }

  delete[] fb;
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
