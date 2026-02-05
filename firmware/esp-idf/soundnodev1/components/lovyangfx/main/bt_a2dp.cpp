#include "bt_a2dp.h"
#include "ring_pcm.h"
#include "mp3_decode.h"

#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"

static const char* TAG = "a2dp";

static bool s_connected = false;
static int s_vol = 80;

static esp_bd_addr_t s_peer = {0};
static bool s_have_peer = false;

static void apply_gain(int16_t* samples, size_t count, int pct) {
  if (pct >= 100) return;
  for (size_t i = 0; i < count; i++) {
    int32_t v = samples[i];
    v = (v * pct) / 100;
    if (v > 32767) v = 32767;
    if (v < -32768) v = -32768;
    samples[i] = (int16_t)v;
  }
}

static int32_t a2dp_data_cb(uint8_t *data, int32_t len) {
  int got = (int)ring_pcm_read(data, len);
  if (got < len) {
    memset(data + got, 0, len - got);
  }
  apply_gain((int16_t*)data, (size_t)len / 2, s_vol);
  return len;
}

static void a2dp_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
  switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
      auto st = param->conn_stat.state;
      if (st == ESP_A2D_CONNECTION_STATE_CONNECTED) {
        s_connected = true;
        ESP_LOGI(TAG, "A2DP connected");
      } else if (st == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
        s_connected = false;
        s_have_peer = false;
        ESP_LOGI(TAG, "A2DP disconnected, restarting discovery");
        esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
      }
    } break;

    case ESP_A2D_AUDIO_STATE_EVT: {
      ESP_LOGI(TAG, "Audio state: %d", param->audio_stat.state);
    } break;

    default:
      break;
  }
}

static void gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  switch (event) {
    case ESP_BT_GAP_DISC_RES_EVT: {
      if (!s_have_peer) {
        memcpy(s_peer, param->disc_res.bda, ESP_BD_ADDR_LEN);
        s_have_peer = true;
        ESP_LOGI(TAG, "Found device, trying connect...");
        esp_bt_gap_cancel_discovery();
        esp_a2d_source_connect(s_peer);
      }
    } break;

    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT: {
      if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
        if (!s_connected && !s_have_peer) {
          ESP_LOGI(TAG, "Discovery stopped, restart");
          esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
        }
      }
    } break;

    default:
      break;
  }
}

bool bt_a2dp_init_soundnode() {
  ESP_ERROR_CHECK(nvs_flash_init());

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
  ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));

  ESP_ERROR_CHECK(esp_bluedroid_init());
  ESP_ERROR_CHECK(esp_bluedroid_enable());

  esp_bt_dev_set_device_name("SoundNode");

  ESP_ERROR_CHECK(esp_bt_gap_register_callback(gap_cb));
  ESP_ERROR_CHECK(esp_a2d_register_callback(a2dp_cb));
  ESP_ERROR_CHECK(esp_a2d_source_register_data_callback(a2dp_data_cb));
  ESP_ERROR_CHECK(esp_a2d_source_init());

  esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

  ESP_LOGI(TAG, "BT ready. Starting discovery for headphones...");
  ESP_ERROR_CHECK(esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0));

  return true;
}

bool bt_a2dp_is_connected() { return s_connected; }

void bt_a2dp_set_volume_pct(int pct) {
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  s_vol = pct;
}
int bt_a2dp_volume_pct() { return s_vol; }
