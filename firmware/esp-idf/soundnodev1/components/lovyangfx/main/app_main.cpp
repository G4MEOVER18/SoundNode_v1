#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sd_mount.h"
#include "playlist.h"
#include "ring_pcm.h"
#include "mp3_decode.h"
#include "bt_a2dp.h"
#include "ui.h"

static const char* TAG = "main";

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "SoundNode boot");

  ui_init();

  if (!sd_mount_init()) {
    ESP_LOGE(TAG, "SD mount failed. Stop.");
    while (true) vTaskDelay(pdMS_TO_TICKS(1000));
  }

  // Ringbuffer: 128KB MVP; if dropouts increase to 256KB
  if (!ring_pcm_init(128 * 1024)) {
    ESP_LOGE(TAG, "Ringbuffer init failed.");
    while (true) vTaskDelay(pdMS_TO_TICKS(1000));
  }

  auto tracks = playlist_scan_mp3("/sdcard/music");
  ESP_LOGI(TAG, "Found %d tracks", (int)tracks.size());
  ui_set_tracks(&tracks);

  mp3_decoder_start(&tracks);
  mp3_decoder_set_volume(80);

  bt_a2dp_init_soundnode();
  bt_a2dp_set_volume_pct(mp3_decoder_volume());

  while (true) {
    ui_loop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
