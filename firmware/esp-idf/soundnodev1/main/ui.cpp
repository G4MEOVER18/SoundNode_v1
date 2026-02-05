#include "ui.h"

#include <string>
#include <vector>

#include <LovyanGFX.hpp>
#include "pin_config.h"

#include "bt_a2dp.h"
#include "mp3_decode.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// -------------------- Stabilität --------------------
// 16 MHz ist fast immer stabil; 27MHz/40MHz machen auf manchen Boards “Rauschzonen”.
#ifndef UI_TFT_WRITE_HZ
  #define UI_TFT_WRITE_HZ 16000000
#endif

// -------------------- Display/Touch Setup --------------------
class LGFX : public lgfx::LGFX_Device {
public:
  lgfx::Panel_ILI9341 _panel;
  lgfx::Bus_SPI       _bus;
  lgfx::Light_PWM     _light;
  lgfx::Touch_XPT2046 _touch;

  LGFX() {
    // ---- BUS (TFT + Touch shared) ----
    {
      auto cfg = _bus.config();
      cfg.spi_host   = HSPI_HOST;
      cfg.spi_mode   = 0;

      cfg.freq_write = UI_TFT_WRITE_HZ;
      cfg.freq_read  = 0;              // READ aus -> stabiler (weil MISO oft Murks)

      cfg.pin_sclk   = (int)RG_TFT_SCLK_PIN;
      cfg.pin_mosi   = (int)RG_TFT_MOSI_PIN;
      cfg.pin_miso   = -1;             // bewusst AUS
      cfg.pin_dc     = (int)RG_TFT_DC_PIN;

      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    // ---- PANEL ----
    {
      auto cfg = _panel.config();
      cfg.pin_cs  = (int)RG_TFT_CS_PIN;
      cfg.pin_rst = (int)RG_TFT_RST_PIN;   // -1 ok

      cfg.panel_width  = 240;
      cfg.panel_height = 320;

      _panel.config(cfg);
    }

    // WICHTIG: Panel setzen, dann Rotation auf dem DEVICE
    setPanel(&_panel);
    setRotation(RG_TFT_ROT);

    // ---- BACKLIGHT ----
    {
      auto cfg = _light.config();
      cfg.pin_bl      = (int)RG_TFT_BL_PIN;
      cfg.invert      = false;
      cfg.freq        = 5000;
      cfg.pwm_channel = 7;
      _light.config(cfg);
      _panel.setLight(&_light);
    }

    // ---- TOUCH ----
    {
      auto cfg = _touch.config();
      cfg.x_min = TOUCH_X_MIN;
      cfg.x_max = TOUCH_X_MAX;
      cfg.y_min = TOUCH_Y_MIN;
      cfg.y_max = TOUCH_Y_MAX;

      cfg.pin_cs  = (int)RG_TOUCH_CS_PIN;
      cfg.pin_int = (int)RG_TOUCH_IRQ_PIN;

      cfg.bus_shared = true;
      cfg.spi_host   = HSPI_HOST;
      cfg.freq       = 2000000;

      _touch.config(cfg);
      _panel.setTouch(&_touch);
    }
  }
};

static LGFX gfx;

// -------------------- UI --------------------
static const int PAD   = 10;
static const int BTN_H = 48;

struct Btn { int x,y,w,h; const char* label; };

static Btn btnPrev{0,0,0,0,"<<"};
static Btn btnPlay{0,0,0,0,"Play"};
static Btn btnNext{0,0,0,0,">>"};
static Btn btnVolD{0,0,0,0,"-"};
static Btn btnVolU{0,0,0,0,"+"};

static const std::vector<std::string>* g_tracks = nullptr;

static bool hit(const Btn& b, int x, int y) {
  return x>=b.x && x<(b.x+b.w) && y>=b.y && y<(b.y+b.h);
}

static std::string basename(const std::string& p) {
  auto pos = p.find_last_of("/\\");
  return (pos == std::string::npos) ? p : p.substr(pos + 1);
}

static void layout_buttons() {
  const int W = (int)gfx.width();
  const int H = (int)gfx.height();

  btnPrev = { PAD,      H - BTN_H - PAD, 52, BTN_H, "<<" };
  btnPlay = { PAD + 60, H - BTN_H - PAD, 60, BTN_H, "Play" };
  btnNext = { PAD + 128,H - BTN_H - PAD, 52, BTN_H, ">>" };

  btnVolD = { PAD,      H - 2*BTN_H - 2*PAD, 52, BTN_H, "-" };
  btnVolU = { PAD + 60, H - 2*BTN_H - 2*PAD, 52, BTN_H, "+" };

  (void)W;
}

static void drawBtn(const Btn& b, bool active=false) {
  gfx.fillRoundRect(b.x, b.y, b.w, b.h, 8, active ? 0x7BEF : 0x39E7);
  gfx.drawRoundRect(b.x, b.y, b.w, b.h, 8, 0xFFFF);
  gfx.setTextColor(0xFFFF);
  gfx.setTextDatum(middle_center);
  gfx.drawString(b.label, b.x + b.w/2, b.y + b.h/2);
}

static void drawHeader() {
  const int W = (int)gfx.width();
  gfx.fillRect(0, 0, W, 44, 0x0000);

  gfx.setTextDatum(top_left);
  gfx.setTextColor(0xFFFF);
  gfx.drawString("SoundNode", 8, 6);

  gfx.setTextDatum(top_right);
  gfx.setTextColor(0xAD55);
  gfx.drawString(bt_a2dp_is_connected() ? "BT: connected" : "BT: waiting", W - 8, 6);

  gfx.setTextDatum(top_left);
  gfx.setTextColor(0xAD55);
  std::string v = "Vol " + std::to_string(mp3_decoder_volume()) + "%";
  gfx.drawString(v.c_str(), 8, 24);
}

static void drawMain() {
  const int W = (int)gfx.width();
  gfx.fillRect(0, 44, W, 160, 0x0000);

  gfx.setTextDatum(top_left);
  gfx.setTextColor(0xFFFF);

  std::string path  = mp3_decoder_current_path();
  std::string title = path.empty() ? "(no track)" : basename(path);

  gfx.drawString("Track:", 8, 54);
  gfx.drawString(title.c_str(), 8, 76);

  gfx.setTextColor(0xAD55);
  gfx.drawString(mp3_decoder_is_playing() ? "State: Playing" : "State: Paused", 8, 108);

  gfx.setTextColor(0xAD55);
  int n = g_tracks ? (int)g_tracks->size() : 0;
  std::string tc = "Tracks: " + std::to_string(n);
  gfx.drawString(tc.c_str(), 8, 132);
}

static void drawAll() {
  gfx.startWrite();
  gfx.fillScreen(0x0000);  // “brutal clear”
  gfx.endWrite();

  drawHeader();
  drawMain();

  drawBtn(btnPrev);
  drawBtn(btnPlay, mp3_decoder_is_playing());
  drawBtn(btnNext);
  drawBtn(btnVolD);
  drawBtn(btnVolU);
}

// -------------------- Exported --------------------
void ui_init() {
  gfx.init();

  // Rotation kommt NUR aus pin_config.h
  gfx.setRotation(RG_TFT_ROT);

  gfx.setTextSize(1);

  // Layout nach Rotation berechnen
  layout_buttons();

  drawAll();
}

void ui_set_tracks(const std::vector<std::string>* tracks) {
  g_tracks = tracks;
  drawAll();
}

void ui_loop() {
  uint16_t x, y;
  if (gfx.getTouch(&x, &y)) {
    vTaskDelay(pdMS_TO_TICKS(80));

    if (hit(btnPrev, (int)x, (int)y)) mp3_decoder_prev();
    else if (hit(btnNext, (int)x, (int)y)) mp3_decoder_next();
    else if (hit(btnPlay, (int)x, (int)y)) mp3_decoder_set_playing(!mp3_decoder_is_playing());
    else if (hit(btnVolD, (int)x, (int)y)) mp3_decoder_set_volume(mp3_decoder_volume() - 5);
    else if (hit(btnVolU, (int)x, (int)y)) mp3_decoder_set_volume(mp3_decoder_volume() + 5);

    bt_a2dp_set_volume_pct(mp3_decoder_volume());
    drawAll();
  }

  static uint32_t last = 0;
  uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
  if (now - last > 1000) {
    last = now;
    drawHeader();
  }
}
