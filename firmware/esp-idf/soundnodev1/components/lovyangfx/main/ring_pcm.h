#pragma once
#include <stddef.h>
#include <stdint.h>

bool ring_pcm_init(size_t capacity_bytes);

size_t ring_pcm_write(const uint8_t* data, size_t len);
size_t ring_pcm_read(uint8_t* out, size_t len);

void ring_pcm_clear();
