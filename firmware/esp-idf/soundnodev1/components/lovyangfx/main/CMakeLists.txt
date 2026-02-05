idf_component_register(
  SRCS
    "app_main.cpp"
    "sd_mount.cpp"
    "playlist.cpp"
    "ring_pcm.cpp"
    "mp3_decode.cpp"
    "bt_a2dp.cpp"
    "ui.cpp"
  INCLUDE_DIRS "."
  REQUIRES fatfs sdmmc driver bt nvs_flash lovyangfx minimp3
)
