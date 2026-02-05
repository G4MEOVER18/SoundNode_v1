#include "sd_mount.h"
#include "pin_config.h"

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

static const char* TAG = "sd";
static const char* MOUNT = "/sdcard";

bool sd_mount_init() {
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024
  };

  sdmmc_card_t* card;
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = VSPI_HOST;

  spi_bus_config_t bus_cfg = {};
  bus_cfg.mosi_io_num = RG_SD_MOSI_PIN;
  bus_cfg.miso_io_num = RG_SD_MISO_PIN;
  bus_cfg.sclk_io_num = RG_SD_SCK_PIN;
  bus_cfg.quadwp_io_num = -1;
  bus_cfg.quadhd_io_num = -1;
  ESP_ERROR_CHECK(spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SPI_DMA_CH_AUTO));

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = RG_SD_CS_PIN;
  slot_config.host_id = (spi_host_device_t)host.slot;

  esp_err_t ret = esp_vfs_fat_sdspi_mount(MOUNT, &host, &slot_config, &mount_config, &card);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SD mount failed: %s", esp_err_to_name(ret));
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  ESP_LOGI(TAG, "Mounted at %s", MOUNT);
  return true;
}

const char* sd_mount_point() { return MOUNT; }
