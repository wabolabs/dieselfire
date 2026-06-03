#include "mock/mock_data.h"
#include "Display/Screens/DieselScreen.h"
#include "Display/Screens/MainStatusScreen.h"
#include "Utility/DebugPort.h"
#include "cfg/DFConfig.h"

#include <SDL2/SDL.h>
#include <lvgl.h>

#include <stdio.h>

// SDL display driver for LVGL
static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* texture = nullptr;
static lv_display_t* display = nullptr;
static uint32_t* sdl_buf = nullptr;
static int frameCount = 0;
static int flushCount = 0;

static void sdl_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  int32_t w = lv_area_get_width(area);
  int32_t h = lv_area_get_height(area);

  // Copy LVGL's ARGB8888 pixels into our SDL framebuffer
  uint32_t* src = (uint32_t*)px_map;
  for (int y = 0; y < h; y++) {
    memcpy(sdl_buf + (area->y1 + y) * TFT_WIDTH + area->x1,
           src + y * w, w * sizeof(uint32_t));
  }

  flushCount++;
  frameCount = flushCount;

  // Blit full framebuffer to SDL
  SDL_UpdateTexture(texture, nullptr, sdl_buf, TFT_WIDTH * 4);
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
  printf("=== DieselFire Simulator ===\n"); fflush(stdout);

  // ── SDL ──────────────────────────────────────────────
  printf("[1/5] SDL_Init...\n"); fflush(stdout);
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("DieselFire Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            TFT_WIDTH, TFT_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) { printf("SDL_CreateWindow failed: %s\n", SDL_GetError()); return 1; }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                              TFT_WIDTH, TFT_HEIGHT);

  sdl_buf = new uint32_t[TFT_WIDTH * TFT_HEIGHT]();

  // Show test pattern to confirm SDL works
  for (int i = 0; i < TFT_WIDTH * TFT_HEIGHT; i++) {
    int x = i % TFT_WIDTH, y = i / TFT_WIDTH;
    sdl_buf[i] = (0xFF << 24) |              // A=255
                 ((x * 255 / TFT_WIDTH) << 16) |  // R gradient
                 ((y * 255 / TFT_HEIGHT) << 8) |  // G gradient
                 0x80;                              // B=128
  }
  SDL_UpdateTexture(texture, nullptr, sdl_buf, TFT_WIDTH * 4);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
  printf("[1/5] SDL ready (test pattern displayed)\n"); fflush(stdout);

  // ── LVGL ─────────────────────────────────────────────
  printf("[2/5] lv_init...\n"); fflush(stdout);
  lv_init();
  DieselScreen::initTheme();

  size_t bufSize = TFT_WIDTH * TFT_HEIGHT * sizeof(lv_color_t);
  printf("  bufSize = %zu bytes (TFT=%dx%d, color=%zu)\n",
         bufSize, TFT_WIDTH, TFT_HEIGHT, sizeof(lv_color_t));
  fflush(stdout);

  uint8_t* buf1 = new uint8_t[bufSize];
  uint8_t* buf2 = new uint8_t[bufSize];

  printf("[3/5] lv_display_create...\n"); fflush(stdout);
  display = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
  lv_display_set_flush_cb(display, sdl_flush_cb);
  lv_display_set_buffers(display, buf1, buf2, bufSize, LV_DISPLAY_RENDER_MODE_FULL);
  lv_display_set_default(display);

  printf("[4/5] input device...\n"); fflush(stdout);
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, sdl_mouse_read);

  printf("[5/5] loading screen...\n"); fflush(stdout);
  auto* mainScreen = new MainStatusScreen();
  mainScreen->onLoad();
  lv_scr_load(mainScreen->getScreen());

  printf("  flush count before refr: %d\n", flushCount); fflush(stdout);
  lv_refr_now(display);
  printf("  flush count after  refr: %d\n", flushCount); fflush(stdout);

  if (flushCount == 0) {
    printf("  WARNING: flush callback was never called!\n"); fflush(stdout);
  }

  // ── Main loop ─────────────────────────────────────────
  uint32_t lastTick = SDL_GetTicks();
  bool running = true;
  uint32_t lastFpsPrint = lastTick;

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

    // Print fps every 2 seconds
    if (now - lastFpsPrint > 2000) {
      printf("  flush=%d  delta=%u\n", flushCount, (unsigned)delta);
      fflush(stdout);
      lastFpsPrint = now;
    }

    uint32_t elapsed = SDL_GetTicks() - now;
    if (elapsed < 16) SDL_Delay(16 - elapsed);
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
