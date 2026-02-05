## High-level architecture

```mermaid
flowchart LR
  BT[Bluetooth A2DP] --> PCM[PCM Ring Buffer]
  SD[SD Card / Playlist] --> DEC[MP3 Decode]
  DEC --> PCM
  PCM --> OUT[Audio Output]
  UI[UI] --> BT
  UI --> SD
```

### Source map
- `main/bt_a2dp.*` – Bluetooth A2DP handling
- `main/mp3_decode.*` – MP3 decode
- `main/ring_pcm.*` – PCM ring buffer
- `main/sd_mount.*` + `main/playlist.*` – SD + playlist
- `main/ui.*` – UI layer
- `main/pin_config.h` – pin definitions
