#pragma once

#include <cstdint>
#include "driver/gpio.h"

// ======================= SD (VSPI) =======================
static constexpr gpio_num_t RG_SD_CS_PIN   = GPIO_NUM_5;
static constexpr gpio_num_t RG_SD_SCK_PIN  = GPIO_NUM_18;
static constexpr gpio_num_t RG_SD_MISO_PIN = GPIO_NUM_19;
static constexpr gpio_num_t RG_SD_MOSI_PIN = GPIO_NUM_23;

// ======================= TFT (HSPI) ======================
static constexpr gpio_num_t RG_TFT_MOSI_PIN = GPIO_NUM_13;
static constexpr gpio_num_t RG_TFT_MISO_PIN = GPIO_NUM_12;   // falls nicht verdrahtet → später -1 behandeln
static constexpr gpio_num_t RG_TFT_SCLK_PIN = GPIO_NUM_14;
static constexpr gpio_num_t RG_TFT_CS_PIN   = GPIO_NUM_15;
static constexpr gpio_num_t RG_TFT_DC_PIN   = GPIO_NUM_2;
static constexpr gpio_num_t RG_TFT_BL_PIN   = GPIO_NUM_21;

// Reset ist bei vielen CYD-Boards fest auf 3V3 → nicht benutzt
static constexpr int RG_TFT_RST_PIN = -1;

// Rotation (kein GPIO, deshalb int!)
static constexpr int RG_TFT_ROT = 1;

// ======================= Touch (XPT2046) =================
static constexpr gpio_num_t RG_TOUCH_CS_PIN  = GPIO_NUM_33;
static constexpr gpio_num_t RG_TOUCH_IRQ_PIN = GPIO_NUM_36;

// Touch-Kalibrierung (keine GPIOs!)
static constexpr int32_t TOUCH_X_MIN = 250;
static constexpr int32_t TOUCH_X_MAX = 3800;
static constexpr int32_t TOUCH_Y_MIN = 250;
static constexpr int32_t TOUCH_Y_MAX = 3800;


// Touch calibration
static constexpr int32_t TOUCH_X_MIN = 250;
static constexpr int32_t TOUCH_X_MAX = 3800;
static constexpr int32_t TOUCH_Y_MIN = 250;
static constexpr int32_t TOUCH_Y_MAX = 3800;
