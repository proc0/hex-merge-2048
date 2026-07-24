#pragma once

#define TARGET_FPS 60
// screen defaults
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
// centimeters/pixel at 1280x720px
#define SCREEN_UNIT 1.0f
#define SCREEN_RESIZE_RATE 300

// TODO: make constexpr
#define UNIT_RATIO (sqrtf(powf(static_cast<float>(SCREEN_WIDTH), 2.0f) + powf(static_cast<float>(SCREEN_HEIGHT), 2.0f)))

#define INTRO_TIME_MS 3000

#define RAYLIB_LOGO_SIZE 200

#define HEX_SIZE 80.0f
// Hex grid extent (half width/height) - 1 (center hex)
// or number of onion hex layers around center hex
#define GRID_EXTENT 2

#define CHIP_FONT_SIZE 52

#define PHASE_COLOR_2 RAYWHITE
#define PHASE_COLOR_4 PINK
#define PHASE_COLOR_8 SKYBLUE
#define PHASE_COLOR_16 LIME
#define PHASE_COLOR_32 GRAY
#define PHASE_COLOR_64 YELLOW
#define PHASE_COLOR_128 MAROON
#define PHASE_COLOR_256 GOLD
#define PHASE_COLOR_512 ORANGE
#define PHASE_COLOR_1024 DARKGRAY
#define PHASE_COLOR_2048 BLACK
#define PHASE_COLOR_4096 MAGENTA

#define CHIP_COLOR_2 Color({ 210, 210, 210, 255 })
#define CHIP_COLOR_4 Color({ 190, 190, 190, 255 })
#define CHIP_COLOR_8 Color({ 170, 170, 170, 255 })
#define CHIP_COLOR_16 Color({ 150, 150, 150, 255 })
#define CHIP_COLOR_32 Color({ 140, 140, 140, 255 })
#define CHIP_COLOR_64 Color({ 120, 120, 120, 255 })
#define CHIP_COLOR_128 Color({ 100, 100, 100, 255 })
#define CHIP_COLOR_256 Color({ 90, 90, 90, 255 })
#define CHIP_COLOR_512 Color({ 60, 60, 60, 255 })
#define CHIP_COLOR_1024 Color({ 40, 40, 40, 255 })
#define CHIP_COLOR_2048 Color({ 0, 0, 0, 255 })
#define CHIP_COLOR_4096 Color({ 0, 0, 0, 255 })
