#include <Arduino.h>
#include "Display/Screens/DieselScreen.h"
#include "Display/Screens/MainStatusScreen.h"
#include "Utility/DebugPort.h"
#include "cfg/DFConfig.h"
#include "vserial.h"
#include "heater_emu.h"
#include "Protocol.h"

#include <SDL2/SDL.h>
#include <lvgl.h>
#include <stdio.h>
#include <cstring>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* texture = nullptr;
static lv_display_t* display = nullptr;

VirtualSerialPair g_serialPair;
// BlueWireSerial is used by bluwire_task.cpp — reference to one end of the pair
VirtualSerial& BlueWireSerial = g_serialPair.a;
HeaterEmulator g_heater;

extern void tickBlueWire(unsigned long timenow);
extern void initBlueWire();
extern void checkBlueWireEvents();
extern CommStates CommState;

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

static uint32_t sdlGetTicks() {
#ifdef __EMSCRIPTEN__
  return SDL_GetTicks();
#else
  return SDL_GetTicks();
#endif
}

static void loopIteration() {
  static uint32_t lastTick = sdlGetTicks();
  uint32_t now = sdlGetTicks();
  int delta = (int)(now - lastTick);
  if (delta < 1) delta = 1;
  lastTick = now;

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
#ifdef __EMSCRIPTEN__
      emscripten_cancel_main_loop();
#endif
      return;
    }
  }

  lv_tick_inc(delta);
  g_heater.tick(delta);
  tickBlueWire(now);
  checkBlueWireEvents();
  lv_task_handler();

  uint32_t elapsed = sdlGetTicks() - now;
  if (elapsed < 16) SDL_Delay(16 - elapsed);
}

#ifndef __EMSCRIPTEN__
static void nativeMainLoop() {
  while (true) loopIteration();
}
#endif

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

  // BlueWire protocol simulation (cooperative — no threads)
  initBlueWire();
  g_heater.begin(&g_serialPair.b);

  lv_refr_now(display);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(loopIteration, 0, 1);
#else
  nativeMainLoop();
#endif

  delete[] fb;
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
