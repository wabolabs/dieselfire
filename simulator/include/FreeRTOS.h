#pragma once
// FreeRTOS adapter for Linux simulator.
// Maps FreeRTOS task/queue/semaphore APIs to C++17 std::thread/mutex/condition_variable.

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>
#include <queue>

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

// ── Task ─────────────────────────────────────────────────────
typedef void* TaskHandle_t;
typedef void (*TaskFunction_t)(void*);

static inline BaseType_t xTaskCreate(TaskFunction_t pvTaskCode,
                                     const char* const pcName,
                                     const uint32_t usStackDepth,
                                     void* const pvParameters,
                                     int uxPriority,
                                     TaskHandle_t* const pvCreatedTask) {
  std::thread* t = new std::thread(pvTaskCode, pvParameters);
  t->detach();
  if (pvCreatedTask) *pvCreatedTask = (void*)t;
  return pdPASS;
}

static inline void vTaskDelay(const TickType_t xTicksToDelay) {
  std::this_thread::sleep_for(std::chrono::milliseconds(xTicksToDelay));
}

static inline TickType_t xTaskGetTickCount() {
  auto now = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

// ── Queue ────────────────────────────────────────────────────
struct Queue {
  std::mutex mtx;
  std::condition_variable cv;
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

static inline BaseType_t xQueueSend(QueueHandle_t q, const void* item, TickType_t timeout) {
  if (!q) return errCOULD_NOT_ALLOCATE;
  std::lock_guard<std::mutex> lock(q->mtx);
  if (q->items.size() >= q->maxCount) return errQUEUE_FULL;
  const uint8_t* data = static_cast<const uint8_t*>(item);
  q->items.emplace(data, data + q->itemSize);
  q->cv.notify_one();
  return pdPASS;
}

static inline BaseType_t xQueueReceive(QueueHandle_t q, void* item, TickType_t timeout) {
  if (!q) return errCOULD_NOT_ALLOCATE;
  std::unique_lock<std::mutex> lock(q->mtx);
  if (q->items.empty()) {
    if (timeout == 0) return errQUEUE_EMPTY;
    q->cv.wait_for(lock, std::chrono::milliseconds(timeout),
                   [q]{ return !q->items.empty(); });
    if (q->items.empty()) return errQUEUE_EMPTY;
  }
  auto& front = q->items.front();
  memcpy(item, front.data(), front.size());
  q->items.pop();
  return pdPASS;
}

static inline BaseType_t xQueueOverwrite(QueueHandle_t q, const void* item) {
  if (!q) return errCOULD_NOT_ALLOCATE;
  std::lock_guard<std::mutex> lock(q->mtx);
  if (q->items.size() >= q->maxCount) {
    while (!q->items.empty()) q->items.pop();
  }
  const uint8_t* data = static_cast<const uint8_t*>(item);
  q->items.emplace(data, data + q->itemSize);
  q->cv.notify_one();
  return pdPASS;
}

// ── Semaphore ────────────────────────────────────────────────
struct Semaphore {
  std::mutex mtx;
  std::condition_variable cv;
  bool given = false;
};

typedef Semaphore* SemaphoreHandle_t;

static inline SemaphoreHandle_t xSemaphoreCreateBinary() {
  return new Semaphore;
}

static inline BaseType_t xSemaphoreGive(SemaphoreHandle_t s) {
  if (!s) return pdFALSE;
  {
    std::lock_guard<std::mutex> lock(s->mtx);
    s->given = true;
  }
  s->cv.notify_one();
  return pdPASS;
}

static inline BaseType_t xSemaphoreTake(SemaphoreHandle_t s, TickType_t timeout) {
  if (!s) return pdFALSE;
  std::unique_lock<std::mutex> lock(s->mtx);
  if (!s->given) {
    if (timeout == portMAX_DELAY) {
      s->cv.wait(lock, [s]{ return s->given; });
    } else if (timeout > 0) {
      s->cv.wait_for(lock, std::chrono::milliseconds(timeout),
                     [s]{ return s->given; });
      if (!s->given) return pdFALSE;
    } else {
      return pdFALSE;
    }
  }
  s->given = false;
  return pdPASS;
}

static inline BaseType_t xSemaphoreTakeFromISR(SemaphoreHandle_t s, BaseType_t* pxHigherPriorityTaskWoken) {
  return xSemaphoreTake(s, 0);
}

// ── Timer ────────────────────────────────────────────────────
typedef void* TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t);

static inline TimerHandle_t xTimerCreate(const char* const pcName,
                                         const TickType_t xPeriod,
                                         const UBaseType_t uxAutoReload,
                                         void* const pvTimerID,
                                         TimerCallbackFunction_t pxCallbackFunction) {
  return (TimerHandle_t)1;
}

static inline BaseType_t xTimerStart(TimerHandle_t xTimer, TickType_t xBlockTime) {
  return pdPASS;
}

static inline BaseType_t xTimerStop(TimerHandle_t xTimer, TickType_t xBlockTime) {
  return pdPASS;
}

static inline BaseType_t xTimerIsTimerActive(TimerHandle_t xTimer) {
  return pdFALSE;
}

// ── Other ────────────────────────────────────────────────────
static inline void taskYIELD() {}
static inline void vTaskDelayUntil(TickType_t* pxPreviousWakeTime, TickType_t xTimeIncrement) {
  std::this_thread::sleep_for(std::chrono::milliseconds(xTimeIncrement));
}
