#include "ui.h"
#include "pin_config.h"
#include "bt_a2dp.h"
#include "mp3_decode.h"

#include <string>
#include <LovyanGFX.hpp>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const int W = 240;
static const int H = 320;
static const int PAD = 10;
static const int BTN_H = 48;

struct Btn { int x,y,w,h; const char* label; };
static Btn btnPrev { PAD, H-BTN_H-PAD, 52, BTN_H, "<<" };
static Btn btnPlay { PAD+60, H-BTN_H-PAD, 60, BTN_H, "Play" };
static Btn btnNext { PAD+128, H-BTN_H-PAD, 52, BTN_H, ">>" };
static Btn btnVolD { PAD, H-2*BTN_H-2*PAD, 52, BTN_H, "-" };
static Btn btnVolU { PAD+60, H-2*BTN_H-2*PAD, 52, BTN_H, "+" };

static const std::vector<std::string>* g_tracks = nullptr;

static std::string basename(const std::string& p) {
  auto pos = p.find_last_of("/\\");
  return (pos == std::string::npos) ? p : p.substr(pos + 1);
}

class LGFX : public lgfx::LGFX_Device {
public:
  lgfx::Panel_ILI9341 _panel;
  lgfx::Bus_SPI       _bus;
  lgfx::Light_PWM     _light;
  lgfx::Touch_XPT2046 _touch;

  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.spi_host   = HSPI_HOST;
      cfg.spi_mode   = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read  = 16000000;
      cfg.pin_sclk   = RG_TFT_SCLK_PIN;
      cfg.pin_mosi   = RG_TFT_MOSI_PIN;
      cfg.pin_miso   = (RG_TFT_MISO_PIN >= 0) ? RG_TFT_MISO_PIN : -1;
      cfg.pin_dc     = RG_TFT_DC_PIN;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs  = RG_TFT_CS_PIN;
      cfg.pin_rst = (RG_TFT_RST_PIN >= 0) ? RG_TFT_RST_PIN : -1;
      cfg.panel_width  = 240;
      cfg.panel_height = 320;
      cfg.rotation = RG_TFT_ROT;
      _panel.config(cfg);
    }
    {
      auto cfg = _light.config();
      cfg.pin_bl = RG_TFT_BL_PIN;
      cfg.invert = false;
      cfg.freq   = 5000;
      cfg.pwm_channel = 7;
      _light.config(cfg);
      _panel.setLight(&_light);
    }
    {
      auto cfg = _touch.config();
      cfg.x_min = TOUCH_X_MIN;
      cfg.x_max = TOUCH_X_MAX;
      cfg.y_min = TOUCH_Y_MIN;
      cfg.y_max = TOUCH_Y_MAX;
      cfg.pin_cs  = RG_TOUCH_CS_PIN;
      cfg.pin_int = RG_TOUCH_IRQ_PIN;
      cfg.bus_shared = true;
      cfg.spi_host = HSPI_HOST;
      cfg.freq = 2000000;
      _touch.config(cfg);
      _panel.setTouch(&_touch);
    }
    setPanel(&_panel);
  }
};

static LGFX gfx;

static bool hit(const Btn& b, int x, int y) {
  return x>=b.x && x<(b.x+b.w) && y>=b.y && y<(b.y+b.h);
}

static void drawBtn(const Btn& b, bool active=false) {
  gfx.fillRoundRect(b.x, b.y, b.w, b.h, 8, active ? 0x7BEF : 0x39E7);
  gfx.drawRoundRect(b.x, b.y, b.w, b.h, 8, 0xFFFF);
  gfx.setTextColor(0xFFFF);
  gfx.setTextDatum(middle_center);
  gfx.drawString(b.label, b.x + b.w/2, b.y + b.h/2);
}

static void drawHeader() {
  gfx.fillRect(0,0,W,44,0x0000);
  gfx.setTextDatum(top_left);
  gfx.setTextColor(0xFFFF);
  gfx.drawString("SoundNode", 8, 6);

  gfx.setTextDatum(top_right);
  gfx.setTextColor(0xAD55);
  gfx.drawString(bt_a2dp_is_connected() ? "BT: connected" : "BT: waiting", W-8, 6);

  gfx.setTextDatum(top_left);
  gfx.setTextColor(0xAD55);
  std::string v = "Vol " + std::to_string(mp3_decoder_volume()) + "%";
  gfx.drawString(v.c_str(), 8, 24);
}

static void drawMain() {
  gfx.fillRect(0,44,W,160,0x0000);
  gfx.setTextDatum(top_left);
  gfx.setTextColor(0xFFFF);

  std::string path = mp3_decoder_current_path();
  std::string title = path.empty() ? "(no track)" : basename(path);

  gfx.drawString("Track:", 8, 54);
  gfx.drawString(title.c_str(), 8, 76);

  gfx.setTextColor(0xAD55);
  gfx.drawString(mp3_decoder_is_playing() ? "State: Playing" : "State: Paused", 8, 108);
}

static void drawAll() {
  gfx.fillScreen(0x0000);
  drawHeader();
  drawMain();
  drawBtn(btnPrev);
  drawBtn(btnPlay, mp3_decoder_is_playing());
  drawBtn(btnNext);
  drawBtn(btnVolD);
  drawBtn(btnVolU);
}

void ui_init() {
  gfx.init();
  gfx.setRotation(RG_TFT_ROT);
  gfx.setTextSize(1);
  drawAll();
}

void ui_set_tracks(const std::vector<std::string>* tracks) {
  g_tracks = tracks;
  (void)g_tracks;
  drawAll();
}

void ui_loop() {
  uint16_t x, y;
  if (gfx.getTouch(&x, &y)) {
    vTaskDelay(pdMS_TO_TICKS(80));

    if (hit(btnPrev, x, y)) mp3_decoder_prev();
    else if (hit(btnNext, x, y)) mp3_decoder_next();
    else if (hit(btnPlay, x, y)) mp3_decoder_set_playing(!mp3_decoder_is_playing());
    else if (hit(btnVolD, x, y)) mp3_decoder_set_volume(mp3_decoder_volume() - 5);
    else if (hit(btnVolU, x, y)) mp3_decoder_set_volume(mp3_decoder_volume() + 5);

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
