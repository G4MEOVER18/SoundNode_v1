# SoundNode v1 – Project Overview

SoundNode v1 is an ESP32 firmware project built with ESP-IDF.

Core modules (based on filenames):
- Bluetooth A2DP audio: `main/bt_a2dp.*`
- MP3 decoding: `main/mp3_decode.*`
- PCM ring buffer: `main/ring_pcm.*`
- SD mount + playlist: `main/sd_mount.*`, `main/playlist.*`
- UI layer: `main/ui.*`
- Pin mapping: `main/pin_config.h`
