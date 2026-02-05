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

  // UI immer zuerst: räumt den Bildschirm auf und zeigt Status
  ui_init();

  // --- SD mount (nicht mehr "Stop." + Endlosschleife) ---
  bool sd_ok = sd_mount_init();
  if (!sd_ok) {
    ESP_LOGE(TAG, "SD mount failed. Continuing without SD.");
  }

  // --- Ringbuffer (ESP32 ohne PSRAM -> klein starten) ---
  bool rb_ok = ring_pcm_init(32 * 1024);
  if (!rb_ok) {
    ESP_LOGE(TAG, "Ringbuffer init failed. Continuing (no audio buffering).");
  }

  // --- Playlist / MP3 nur wenn SD + Ringbuffer ok sind ---
  std::vector<std::string> tracks;

  if (sd_ok) {
    tracks = playlist_scan_mp3("/sdcard/music");
    ESP_LOGI(TAG, "Found %d tracks", (int)tracks.size());
    ui_set_tracks(&tracks);
  } else {
    ESP_LOGW(TAG, "No SD -> no tracks.");
    ui_set_tracks(&tracks); // leere Liste anzeigen
  }

  if (sd_ok && rb_ok && !tracks.empty()) {
    mp3_decoder_start(&tracks);
    mp3_decoder_set_volume(80);
  } else {
    ESP_LOGW(TAG, "Decoder not started (sd_ok=%d rb_ok=%d tracks=%d).",
             (int)sd_ok, (int)rb_ok, (int)tracks.size());
  }

  // --- Bluetooth kann unabhängig laufen (UI + pairing testen) ---
  bt_a2dp_init_soundnode();
  bt_a2dp_set_volume_pct(mp3_decoder_volume());

  while (true) {
    ui_loop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
