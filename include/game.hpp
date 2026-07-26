#pragma once

#include "type.hpp"
#include "window.hpp"
#include "text.hpp"

// #define CHIP_COLOR_COUNT 12
// static inline constexpr Color chipColors[CHIP_COLOR_COUNT] = {
//     CHIP_COLOR_2,
//     CHIP_COLOR_4,
//     CHIP_COLOR_8,
//     CHIP_COLOR_16,
//     CHIP_COLOR_32,
//     CHIP_COLOR_64,
//     CHIP_COLOR_128,
//     CHIP_COLOR_256,
//     CHIP_COLOR_512,
//     CHIP_COLOR_1024,
//     CHIP_COLOR_2048,
//     CHIP_COLOR_4096,
// };

class Game : public Layer {
    Font bgTitleFont;
    Font titleFont;

    const char* title = GAME_TITLE;
    int titleFontSize = TITLE_FONT_SIZE;
    float titleX;
    float titleY;
    
    int bgTitleFontSize = TITLE_BG_FONT_SIZE;
    float bgTitleX;
    float bgTitleY;

    const char* titleHint = TITLE_HINT;
    int titleHintFontSize = TITLE_HINT_FONT_SIZE;
    float titleHintX;
    float titleHintY;

    GameState meta = defaultGameState;

    const Window& window;
    TimerId gameTimerId;

    int moveCount = 0;
    bool paused = false;

public:
    Game(const Window& window): window(window) {}
    ~Game() = default;

    void (Game::*render)() const = &Game::renderUnit;
    GameState (Game::*update)(InputEvent, WorldState) = &Game::updateUnit;
    
    void load();
    void start();
    void restart();
    void continueGame();

    void renderUnit() const {};
    void renderGame() const;
    void renderMain() const;
    void renderTitle() const;
    void renderTitleBg() const;

    GameState updateUnit(InputEvent, WorldState) { return defaultGameState; };
    GameState updateMain(InputEvent, WorldState);
    GameState updateGame(InputEvent, WorldState);
    void updateTitle();

    bool finished() const;
    void transition(State::App, State::Screen);
    void resizeTitle();
    void resize(int height, int width) override;
    void unload();
};
