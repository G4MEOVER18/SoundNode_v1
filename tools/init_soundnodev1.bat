@echo off
setlocal enabledelayedexpansion

set "ROOT=C:\Espressif\Projekte\soundnodev1"

echo [*] Creating project at: "%ROOT%"

REM --- Create folders ---
mkdir "%ROOT%" 2>nul
mkdir "%ROOT%\main" 2>nul
mkdir "%ROOT%\components" 2>nul
mkdir "%ROOT%\components\lovyangfx" 2>nul
mkdir "%ROOT%\components\minimp3" 2>nul
mkdir "%ROOT%\components\minimp3\include" 2>nul

REM --- Create root files (placeholders) ---
if not exist "%ROOT%\CMakeLists.txt" (
  type nul > "%ROOT%\CMakeLists.txt"
)
if not exist "%ROOT%\sdkconfig.defaults" (
  type nul > "%ROOT%\sdkconfig.defaults"
)

REM --- Component CMake files (placeholders) ---
if not exist "%ROOT%\components\lovyangfx\CMakeLists.txt" (
  type nul > "%ROOT%\components\lovyangfx\CMakeLists.txt"
)
if not exist "%ROOT%\components\minimp3\CMakeLists.txt" (
  type nul > "%ROOT%\components\minimp3\CMakeLists.txt"
)

REM --- Main files (placeholders) ---
set FILES= ^
  CMakeLists.txt ^
  app_main.cpp ^
  pin_config.h ^
  sd_mount.cpp ^
  sd_mount.h ^
  playlist.cpp ^
  playlist.h ^
  ring_pcm.cpp ^
  ring_pcm.h ^
  mp3_decode.cpp ^
  mp3_decode.h ^
  bt_a2dp.cpp ^
  bt_a2dp.h ^
  ui.cpp ^
  ui.h

for %%F in (%FILES%) do (
  if not exist "%ROOT%\main\%%F" (
    type nul > "%ROOT%\main\%%F"
  )
)

echo.
echo [OK] Structure created.
echo.
echo Next:
echo  1) Put LovyanGFX sources into: "%ROOT%\components\lovyangfx\"
echo  2) Put minimp3.h + minimp3_ex.h into: "%ROOT%\components\minimp3\include\"
echo  3) Paste the code into the created placeholder files.
echo.
pause
endlocal
