#pragma once
// FreeRTOS adapter for single-threaded cooperative model (native + WASM).
// All queues/semaphores are lock-free — no threads, no mutex, no atomics.

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <vector>

// Standard FreeRTOS types
typedef uint32_t TickType_t;
typedef int BaseType_t;
typedef unsigned int UBaseType_t;

#define pdFALSE 0
#define pdPASS 1
#define pdTRUE 1
#define errQUEUE_FULL 0
#define errQUEUE_EMPTY 0
#define errCOULD_NOT_ALLOCATE 0
#define portMAX_DELAY ((TickType_t)0xFFFFFFFFUL)
#define portTICK_PERIOD_MS 1

// ── Task (cooperative — stores callback, no thread) ─────────
typedef void* TaskHandle_t;
typedef void (*TaskFunction_t)(void*);

static inline BaseType_t xTaskCreate(TaskFunction_t fn, const char*, uint32_t,
                                     void* pv, int, TaskHandle_t*) {
  (void)fn; (void)pv;
  return pdPASS;
}

static inline void vTaskDelay(TickType_t) {}

// ── Queue (lock-free, single consumer/producer) ──────────────
struct Queue {
  std::queue<std::vector<uint8_t>> items;
  size_t maxCount;
  size_t itemSize;
};

typedef Queue* QueueHandle_t;

static inline QueueHandle_t xQueueCreate(int count, int size) {
  Queue* q = new Queue;
  q->maxCount = count;
  q->itemSize = size;
  return q;
}

static inline BaseType_t xQueueSend(QueueHandle_t q, const void* item, TickType_t) {
  if (!q || q->items.size() >= q->maxCount) return errQUEUE_FULL;
  const uint8_t* data = static_cast<const uint8_t*>(item);
  q->items.emplace(data, data + q->itemSize);
  return pdPASS;
}

static inline BaseType_t xQueueReceive(QueueHandle_t q, void* item, TickType_t timeout) {
  if (!q || q->items.empty()) return errQUEUE_EMPTY;
  auto& front = q->items.front();
  memcpy(item, front.data(), front.size());
  q->items.pop();
  return pdPASS;
}

// ── Semaphore (lock-free flag) ──────────────────────────────
struct Semaphore {
  bool given = false;
};

typedef Semaphore* SemaphoreHandle_t;

static inline SemaphoreHandle_t xSemaphoreCreateBinary() {
  return new Semaphore;
}

static inline BaseType_t xSemaphoreGive(SemaphoreHandle_t s) {
  if (s) s->given = true;
  return pdPASS;
}

static inline BaseType_t xSemaphoreTake(SemaphoreHandle_t s, TickType_t timeout) {
  if (!s || !s->given) return pdFALSE;
  s->given = false;
  return pdPASS;
}

// ── Timer (no-op — cooperative model manages timing directly) ─
typedef void* TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t);

static inline TimerHandle_t xTimerCreate(const char*, TickType_t, UBaseType_t,
                                         void*, TimerCallbackFunction_t) {
  return (TimerHandle_t)1;
}
static inline BaseType_t xTimerStart(TimerHandle_t, TickType_t) { return pdPASS; }
static inline BaseType_t xTimerStop(TimerHandle_t, TickType_t) { return pdPASS; }
static inline BaseType_t xTimerIsTimerActive(TimerHandle_t) { return pdFALSE; }
