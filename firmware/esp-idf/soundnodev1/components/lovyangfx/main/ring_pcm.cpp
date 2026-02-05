#include "ring_pcm.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

static RingbufHandle_t rb = nullptr;

bool ring_pcm_init(size_t capacity_bytes) {
  rb = xRingbufferCreate(capacity_bytes, RINGBUF_TYPE_BYTEBUF);
  return rb != nullptr;
}

void ring_pcm_clear() {
  if (!rb) return;
  while (true) {
    size_t sz = 0;
    void* item = xRingbufferReceiveUpTo(rb, &sz, 0, 4096);
    if (!item) break;
    vRingbufferReturnItem(rb, item);
  }
}

size_t ring_pcm_write(const uint8_t* data, size_t len) {
  if (!rb || len == 0) return 0;
  BaseType_t ok = xRingbufferSend(rb, (void*)data, len, 0);
  return ok == pdTRUE ? len : 0;
}

size_t ring_pcm_read(uint8_t* out, size_t len) {
  if (!rb || len == 0) return 0;

  size_t got_total = 0;
  while (got_total < len) {
    size_t item_size = 0;
    uint8_t* item = (uint8_t*)xRingbufferReceiveUpTo(rb, &item_size, 0, len - got_total);
    if (!item) break;

    memcpy(out + got_total, item, item_size);
    got_total += item_size;
    vRingbufferReturnItem(rb, item);
  }
  return got_total;
}
