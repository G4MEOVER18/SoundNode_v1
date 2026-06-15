# SoundNode v1 (ESP32 / ESP-IDF)

This repository contains the **SoundNode v1** ESP-IDF project, structured for GitHub and Windows-friendly paths.

## Layout
- `firmware/esp-idf/soundnodev1/` – ESP-IDF project (sources + components)
- `tools/` – helper scripts (Windows)
- `docs/` – overview + diagrams + key module map

## Build & flash (ESP-IDF)
Open a terminal in `firmware/esp-idf/soundnodev1`:

- `idf.py set-target esp32`
- `idf.py menuconfig`
- `idf.py build`
- `idf.py -p <PORT> flash monitor`

## Note about build artifacts
The original `build/` directory was excluded to avoid Windows “path too long” issues and large generated files.
It is safe to regenerate with `idf.py build`.

---

## Kontakt

**Entwickler:** Yanis Ameseder · **E-Mail:** [g4me.over.18@gmail.com](mailto:g4me.over.18@gmail.com)
