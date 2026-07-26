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

#define TITLE_FONT_SIZE 150
#define TITLE_HINT_FONT_SIZE 32
#define TITLE_BG_FONT_SIZE 700

#define HEX_SIZE 72.0f
// Hex grid extent (half width/height) - 1 (center hex)
// or number of onion hex layers around center hex
#define GRID_EXTENT 2

#define WORLD_COLOR_PRIMARY_BG DARKGRAY
#define WORLD_COLOR_SECONDARY_BG LIGHTGRAY

#define CHIP_FONT_SIZE 52

#define CHIP_COLOR_2 LIGHTGRAY
#define CHIP_COLOR_4 PINK
#define CHIP_COLOR_8 SKYBLUE
#define CHIP_COLOR_16 YELLOW
#define CHIP_COLOR_32 LIME
#define CHIP_COLOR_64 BLUE
#define CHIP_COLOR_128 MAROON
#define CHIP_COLOR_256 ORANGE
#define CHIP_COLOR_512 GOLD
#define CHIP_COLOR_1024 RED
#define CHIP_COLOR_2048 MAGENTA
#define CHIP_COLOR_4096 BLACK

#define PHASE_COLOR_2 RAYWHITE
#define PHASE_COLOR_4 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_8 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_16 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_32 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_64 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_128 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_256 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_512 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_1024 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_2048 Color({ 230, 230, 230, 255 })
#define PHASE_COLOR_4096 Color({ 230, 230, 230, 255 })
