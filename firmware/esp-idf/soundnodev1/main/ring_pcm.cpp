#include "ring_pcm.h"

#include <string.h>
#include <algorithm>

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

static const char* TAG = "ring_pcm";
static RingbufHandle_t rb = nullptr;

// Für ESP32 ohne PSRAM: klein anfangen.
// 32 KB ist stabil, 48/64 KB geht manchmal, je nach BT+UI RAM.
static constexpr size_t RB_DEFAULT = 32 * 1024;
static constexpr size_t RB_MAX_SAFE = 64 * 1024;

bool ring_pcm_init(size_t capacity_bytes) {
  // Schon initialisiert?
  if (rb) return true;

  // 0 oder Unsinn -> Default
  if (capacity_bytes == 0) capacity_bytes = RB_DEFAULT;

  // Safety clamp (sonst scheitert malloc/ringbuffer)
  capacity_bytes = std::min(capacity_bytes, RB_MAX_SAFE);

  ESP_LOGI(TAG, "init capacity=%u bytes (free8=%u, freeheap=%u)",
           (unsigned)capacity_bytes,
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
           (unsigned)esp_get_free_heap_size());

  rb = xRingbufferCreate(capacity_bytes, RINGBUF_TYPE_BYTEBUF);
  if (!rb) {
    ESP_LOGE(TAG, "xRingbufferCreate failed (capacity=%u, free8=%u, freeheap=%u)",
             (unsigned)capacity_bytes,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)esp_get_free_heap_size());
    return false;
  }
  return true;
}

void ring_pcm_clear() {
  if (!rb) return;

  while (true) {
    size_t sz = 0;
    void* item = xRingbufferReceive(rb, &sz, 0); // 0 ticks: non-blocking
    if (!item) break;
    vRingbufferReturnItem(rb, item);
  }
}

size_t ring_pcm_write(const uint8_t* data, size_t len) {
  if (!rb || !data || len == 0) return 0;

  // Non-blocking, aber 1 kurzer Retry hilft gegen "fast full" Situationen.
  if (xRingbufferSend(rb, (void*)data, len, 0) == pdTRUE) return len;

  // kurzer Yield + Retry
  vTaskDelay(1);
  if (xRingbufferSend(rb, (void*)data, len, 0) == pdTRUE) return len;

  return 0;
}

size_t ring_pcm_read(uint8_t* out, size_t len) {
  if (!rb || !out || len == 0) return 0;

  size_t got_total = 0;

  while (got_total < len) {
    size_t item_size = 0;

    // Nur so viel holen wie noch gebraucht wird, non-blocking
    uint8_t* item = (uint8_t*)xRingbufferReceiveUpTo(
      rb, &item_size, 0, len - got_total
    );

    if (!item) break;

    memcpy(out + got_total, item, item_size);
    got_total += item_size;

    vRingbufferReturnItem(rb, item);
  }

  return got_total;
}
