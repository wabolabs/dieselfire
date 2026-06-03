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
static uint32_t* fb = nullptr;
int frameCount = 0;

static void sdl_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  int32_t w = lv_area_get_width(area);
  int32_t h = lv_area_get_height(area);
  // LVGL uses BGRA byte order on little-endian, which matches SDL_PIXELFORMAT_ARGB8888
  uint32_t* src = (uint32_t*)px_map;
  uint32_t* dst = sdl_buf + area->y1 * TFT_WIDTH + area->x1;
  for (int y = 0; y < h; y++) {
    memcpy(dst + y * TFT_WIDTH, src + y * w, w * 4);
  }
  frameCount++;
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
  printf("Starting...\n");
  fflush(stdout);
  printf("SDL_Init...\n");
  fflush(stdout);
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }
  printf("SDL_Init OK\n");
  fflush(stdout);

  window = SDL_CreateWindow("DieselFire Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            TFT_WIDTH, TFT_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) {
    printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
    return 1;
  }
  printf("SDL window created\n");

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                              TFT_WIDTH, TFT_HEIGHT);
  sdl_buf = new uint32_t[TFT_WIDTH * TFT_HEIGHT]();
  memset(sdl_buf, 0, TFT_WIDTH * TFT_HEIGHT * 4);
  printf("SDL resources ready\n");

  // Init LVGL
  lv_init();
  DieselScreen::initTheme();
  printf("LVGL initialized\n");

  size_t bufSize = TFT_WIDTH * TFT_HEIGHT * 4;
  uint8_t* buf1 = new uint8_t[bufSize];
  uint8_t* buf2 = new uint8_t[bufSize];

  display = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
  lv_display_set_flush_cb(display, sdl_flush_cb);
  lv_display_set_buffers(display, buf1, buf2, bufSize, LV_DISPLAY_RENDER_MODE_DIRECT);
  printf("LVGL display created\n");

  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, sdl_mouse_read);
  printf("LVGL input device created\n");

  // Load main screen and force initial render
  auto* mainScreen = new MainStatusScreen();
  mainScreen->onLoad();
  lv_scr_load(mainScreen->getScreen());
  printf("Screen loaded, starting initial render...\n");
  fflush(stdout);

  lv_refr_now(display);
  printf("Initial render complete\n");
  fflush(stdout);

  // Main loop
  uint32_t lastTick = SDL_GetTicks();
  bool running = true;

  while (running) {
    uint32_t now = SDL_GetTicks();
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
    }

    // Always call lv_tick_inc with the delta
    uint32_t delta = now - lastTick;
    lastTick = now;
    lv_tick_inc(delta);

    // Update simulation
    updateSimulation(delta);

    lv_task_handler();

    // Cap at ~60fps
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
