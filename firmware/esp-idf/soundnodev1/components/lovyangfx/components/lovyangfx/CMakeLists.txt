# LovyanGFX wird als Component eingebunden.
# Lege den LovyanGFX-Repo-Inhalt in diesen Ordner (typisch: src/, lgfx/, etc.).
# Passe die Pfade an, falls dein LovyanGFX-Checkout anders aufgebaut ist.

file(GLOB_RECURSE LGFX_SOURCES
  "src/*.cpp"
  "src/*.c"
)

idf_component_register(
  SRCS ${LGFX_SOURCES}
  INCLUDE_DIRS "src" "src/lgfx"
)
