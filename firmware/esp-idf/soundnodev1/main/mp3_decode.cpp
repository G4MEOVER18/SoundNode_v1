#include "mp3_decode.h"
#include "ring_pcm.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
  #include "minimp3.h"
  #include "minimp3_ex.h"
}

static const char* TAG = "mp3";

static const std::vector<std::string>* g_tracks = nullptr;
static int g_idx = 0;
static bool g_playing = false;
static int g_volume = 80;

static TaskHandle_t task_h = nullptr;

static void task_fn(void*) {
  mp3dec_t dec;
  mp3dec_init(&dec);

  const int IN_SZ = 2048;
  uint8_t inbuf[IN_SZ];

  while (true) {
    if (!g_tracks || g_tracks->empty()) {
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }
    if (!g_playing) {
      vTaskDelay(pdMS_TO_TICKS(30));
      continue;
    }

    std::string path = (*g_tracks)[g_idx];
    ESP_LOGI(TAG, "Open: %s", path.c_str());
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) {
      ESP_LOGE(TAG, "fopen failed");
      g_playing = false;
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    ring_pcm_clear();

    while (g_playing) {
      int r = (int)fread(inbuf, 1, IN_SZ, f);
      if (r <= 0) break; // EOF

      mp3dec_frame_info_t info;
      int16_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
      int samples = mp3dec_decode_frame(&dec, inbuf, r, pcm, &info);

      if (info.frame_bytes == 0) {
        vTaskDelay(pdMS_TO_TICKS(1));
        continue;
      }

      if (samples > 0) {
        int ch = info.channels == 0 ? 2 : info.channels;

        if (ch == 1) {
          static int16_t stereo[MINIMP3_MAX_SAMPLES_PER_FRAME * 2];
          for (int i = 0; i < samples; i++) {
            stereo[2*i] = pcm[i];
            stereo[2*i + 1] = pcm[i];
          }
          size_t bytes = (size_t)samples * 2 * 2;
          (void)ring_pcm_write((uint8_t*)stereo, bytes);
        } else {
          size_t bytes = (size_t)samples * (size_t)ch * 2;
          (void)ring_pcm_write((uint8_t*)pcm, bytes);
        }
      }

      vTaskDelay(pdMS_TO_TICKS(1));
    }

    fclose(f);

    if (g_playing) {
      g_idx = (g_idx + 1) % (int)g_tracks->size();
    }
  }
}

void mp3_decoder_start(const std::vector<std::string>* tracks) {
  g_tracks = tracks;
  g_idx = 0;
  g_playing = (tracks && !tracks->empty());

  if (!task_h) {
    xTaskCreatePinnedToCore(task_fn, "mp3_decode", 8192, nullptr, 5, &task_h, 1);
  }
}

void mp3_decoder_set_playing(bool on) { g_playing = on; }
bool mp3_decoder_is_playing() { return g_playing; }

void mp3_decoder_next() {
  if (!g_tracks || g_tracks->empty()) return;
  g_idx = (g_idx + 1) % (int)g_tracks->size();
  ring_pcm_clear();
  g_playing = true;
}
void mp3_decoder_prev() {
  if (!g_tracks || g_tracks->empty()) return;
  g_idx = (g_idx - 1 + (int)g_tracks->size()) % (int)g_tracks->size();
  ring_pcm_clear();
  g_playing = true;
}

std::string mp3_decoder_current_path() {
  if (!g_tracks || g_tracks->empty()) return {};
  return (*g_tracks)[g_idx];
}

void mp3_decoder_set_volume(int pct) {
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  g_volume = pct;
}
int mp3_decoder_volume() { return g_volume; }
