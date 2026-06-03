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
  // LVGL renders in ARGB8888 (4 bytes/pixel), matching SDL_PIXELFORMAT_ARGB8888.
  size_t pixelBytes = sizeof(lv_color32_t);
  for (int y = 0; y < h; y++) {
    memcpy(sdl_buf + (area->y1 + y) * TFT_WIDTH + area->x1,
           px_map + y * w * pixelBytes, w * pixelBytes);
  }

  flushCount++;
  frameCount = flushCount;

  // Save first frame to PPM for debugging
  if (flushCount == 1) {
    FILE* f = fopen("/tmp/lvgl_frame.ppm", "wb");
    if (f) {
      fprintf(f, "P6\n%d %d\n255\n", TFT_WIDTH, TFT_HEIGHT);
      for (int y = 0; y < TFT_HEIGHT; y++) {
        for (int x = 0; x < TFT_WIDTH; x++) {
          uint32_t c = sdl_buf[y * TFT_WIDTH + x];
          putc((c >> 16) & 0xFF, f); // R
          putc((c >> 8) & 0xFF, f);  // G
          putc(c & 0xFF, f);         // B
        }
      }
      fclose(f);
      printf("  saved /tmp/lvgl_frame.ppm\n"); fflush(stdout);
    }
  }

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
  printf("  LV_COLOR_DEPTH=%d, sizeof(lv_color_t)=%zu, sizeof(lv_color32_t)=%zu\n",
         LV_COLOR_DEPTH, sizeof(lv_color_t), sizeof(lv_color32_t));
  fflush(stdout);

  // Display buffer: ARGB8888 (4 bytes/pixel). lv_color_t is only 3 bytes,
  // but with LV_COLOR_DEPTH=32 the render pixels are lv_color32_t (4 bytes).
  size_t bpp = sizeof(lv_color32_t);
  size_t bufSize = TFT_WIDTH * TFT_HEIGHT * bpp;
  printf("  bufSize = %zu bytes (TFT=%dx%d, bpp=%zu)\n",
         bufSize, TFT_WIDTH, TFT_HEIGHT, bpp);
  fflush(stdout);

  uint8_t* buf1 = new uint8_t[bufSize];
  uint8_t* buf2 = new uint8_t[bufSize];

  printf("[3/5] lv_display_create...\n"); fflush(stdout);
  display = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
  fflush(stdout);
  printf("  lv_display_create returned %p\n", (void*)display); fflush(stdout);
  if (display) {
    lv_display_set_flush_cb(display, sdl_flush_cb);
    printf("  flush cb set\n"); fflush(stdout);
    lv_display_set_buffers(display, buf1, buf2, bufSize, LV_DISPLAY_RENDER_MODE_FULL);
    printf("  buffers set\n"); fflush(stdout);
    lv_display_set_default(display);
    printf("  display set default\n"); fflush(stdout);
  } else {
    printf("  ERROR: display creation failed!\n"); fflush(stdout);
  }

  // Theme must be initialized AFTER a display exists
  DieselScreen::initTheme();
  printf("  theme ready\n"); fflush(stdout);

  printf("[4/5] input device...\n"); fflush(stdout);
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, sdl_mouse_read);

  printf("[5/5] loading screen...\n"); fflush(stdout);

  // Minimal test: solid amber screen
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_size(scr, TFT_WIDTH, TFT_HEIGHT);
  lv_obj_set_style_bg_color(scr, lv_palette_main(LV_PALETTE_ORANGE), 0);
  lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

  // Simple label top-left
  lv_obj_t* lbl = lv_label_create(scr);
  lv_label_set_text(lbl, "DieselFire");
  lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
  lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 8, 8);

  // Second label bottom-right
  lv_obj_t* lbl2 = lv_label_create(scr);
  lv_label_set_text(lbl2, "v1.0");
  lv_obj_set_style_text_color(lbl2, lv_color_white(), 0);
  lv_obj_align(lbl2, LV_ALIGN_BOTTOM_RIGHT, -8, -8);

  lv_scr_load(scr);

  printf("  flush count before refr: %d\n", flushCount); fflush(stdout);
  lv_refr_now(display);
  printf("  flush count after  refr: %d\n", flushCount); fflush(stdout);

  if (flushCount == 0) {
    printf("  WARNING: flush callback was never called!\n"); fflush(stdout);
  }

  // Verify SDL still works after LVGL: show a red/blue pattern
  printf("  SDL direct draw test...\n"); fflush(stdout);
  for (int y = 0; y < TFT_HEIGHT; y++) {
    for (int x = 0; x < TFT_WIDTH; x++) {
      if (x < TFT_WIDTH/2) {
        sdl_buf[y * TFT_WIDTH + x] = 0xFFFF0000; // ARGB red
      } else {
        sdl_buf[y * TFT_WIDTH + x] = 0xFF0000FF; // ARGB blue
      }
    }
  }
  SDL_UpdateTexture(texture, nullptr, sdl_buf, TFT_WIDTH * 4);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
  printf("  SDL direct draw: half red, half blue\n"); fflush(stdout);
  SDL_Delay(1000); // hold for 1 second

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
