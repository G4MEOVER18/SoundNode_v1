#pragma once

#include "driver/gpio.h"

// ====================== TFT (HSPI) ======================
// ILI9341 / ILI9342 SPI (typisch: CYD-Boards, 240x320)

static constexpr gpio_num_t RG_TFT_SCLK_PIN = GPIO_NUM_18;
static constexpr gpio_num_t RG_TFT_MOSI_PIN = GPIO_NUM_23;
static constexpr gpio_num_t RG_TFT_CS_PIN   = GPIO_NUM_15;
static constexpr gpio_num_t RG_TFT_DC_PIN   = GPIO_NUM_2;
static constexpr gpio_num_t RG_TFT_BL_PIN   = GPIO_NUM_21;

// Optional / oft nicht vorhanden:
static constexpr int RG_TFT_MISO_PIN = -1;   // TFT-MISO meistens unzuverlässig -> AUS
static constexpr int RG_TFT_RST_PIN  = -1;   // viele Boards haben Reset fest verdrahtet

// Rotation (LovyanGFX): 0..3 normal, 4..7 gespiegelt
// (Du hast aktuell mit 4/5 experimentiert -> genau hier einstellen)
static constexpr int RG_TFT_ROT = 4;

// ====================== SD-Karte (VSPI) ======================
// CYD-typische SD-Belegung (eigener CS, MISO für Kartenlesung). Bei
// abweichender Hardware hier anpassen — am Gerät verifizieren.
static constexpr gpio_num_t RG_SD_SCK_PIN  = GPIO_NUM_18;
static constexpr gpio_num_t RG_SD_MOSI_PIN = GPIO_NUM_23;
static constexpr gpio_num_t RG_SD_MISO_PIN = GPIO_NUM_19;
static constexpr gpio_num_t RG_SD_CS_PIN   = GPIO_NUM_5;

// ====================== Touch (XPT2046) ======================
// Touch ist am selben SPI-Bus, eigener CS + optional IRQ

static constexpr gpio_num_t RG_TOUCH_CS_PIN  = GPIO_NUM_33;
static constexpr gpio_num_t RG_TOUCH_IRQ_PIN = GPIO_NUM_36; // -1 wenn nicht genutzt / nicht vorhanden

// Touch-Kalibrierung (musst du ggf. feinjustieren)
static constexpr int TOUCH_X_MIN = 200;
static constexpr int TOUCH_X_MAX = 3800;
static constexpr int TOUCH_Y_MIN = 200;
static constexpr int TOUCH_Y_MAX = 3800;
