#include "game.hpp"

#include "index.h"
#include "config.hpp"
#include "type.hpp"

#include "raylib.h"

void Game::load() {
    
    titleFontSize = window.scale(TITLE_FONT_SIZE);
    titleFont = LoadFontEx(PATH_ASSET(URI_FONT_TURRENT_EXTRA_BOLD), titleFontSize, 0, 400);
    SetTextureFilter(titleFont.texture, TEXTURE_FILTER_BILINEAR);

    bgTitleFontSize = window.scale(TITLE_BG_FONT_SIZE);
    bgTitleFont = LoadFontEx(PATH_ASSET(URI_FONT_HEX_GIRLFRIEND), bgTitleFontSize, 0, 400);
    SetTextureFilter(bgTitleFont.texture, TEXTURE_FILTER_BILINEAR);

    resizeTitle();
}

void Game::start() {
    meta.state = State::Game::PLAY;
    gameTimerId = window.timer.startWatch();
    meta.totalTimeId = gameTimerId;
}

void Game::restart() {
    meta.score = 0;
    meta.state = State::Game::PLAY;
    meta.moveCount = 0;
    gameTimerId = window.timer.startWatch();
    meta.totalTimeId = gameTimerId;
}

void Game::continueGame() {
    meta.state = State::Game::STAY;
}

void Game::renderMain() const {
    renderTitleBg();
}

void Game::renderGame() const {

}

void Game::renderTitle() const {
    ClearBackground(BLACK);
    renderTitleBg();
    DrawText(titleHint, titleHintX-2, titleHintY+2, titleHintFontSize, BLACK);
    DrawText(titleHint, titleHintX, titleHintY, titleHintFontSize, RAYWHITE);
}

void Game::renderTitleBg() const {
    DrawRectangleGradientEx({ 0, 0, window.widthf, window.heightf }, CHIP_COLOR_4096, CHIP_COLOR_2048, DARKGRAY, CHIP_COLOR_2048);
    DrawTextEx(bgTitleFont, title, { bgTitleX, bgTitleY }, bgTitleFontSize, 0, BLACK);
    DrawTextEx(bgTitleFont, title, { bgTitleX, bgTitleY }, bgTitleFontSize-5, 0, BEIGE);

    DrawTextEx(titleFont, title, { titleX+10, titleY-10 }, titleFontSize, 0, BLACK);
    DrawTextEx(titleFont, title, { titleX+5, titleY-5 }, titleFontSize, 0, BLACK);
    DrawTextEx(titleFont, title, { titleX, titleY }, titleFontSize, 0, RAYWHITE);
}

GameState Game::updateMain(InputEvent, WorldState){
    updateTitle();
    return meta;
}

GameState Game::updateGame(InputEvent inputEvent, WorldState worldState){
    if (meta.state != State::Game::PLAY && meta.state != State::Game::STAY) return meta;
    
    if (paused) {
        meta.state = State::Game::PAUSE;
        return meta;
    }
 
    if (worldState.moveCount > meta.moveCount) {
        meta.score = worldState.mergedValue;
        meta.moveCount = worldState.moveCount;
    }

    if (meta.state != State::Game::STAY && worldState.maxValue == 2048) {
        window.timer.stopWatch(gameTimerId);
        meta.state = State::Game::WIN;
    } else if (worldState.gridlock) {
        window.timer.stopWatch(gameTimerId);
        meta.state = State::Game::OVER;
    }

    return meta;
}

void Game::updateTitle() {
    float frameTime = GetFrameTime();
    bgTitleX += 20.0f*frameTime;
    bgTitleY += 8.0f*frameTime;

    if (bgTitleY > window.heightf || bgTitleX > window.widthf) {
        float bgTitleTextSize = MeasureText(title, bgTitleFontSize);
        bgTitleX = -bgTitleTextSize;
        bgTitleY = -bgTitleFontSize;       
    }
}

bool Game::finished() const {
    return meta.state == State::Game::WIN || meta.state == State::Game::OVER;
}

void Game::transition(State::App appState, State::Screen screen) {
    switch(screen) {
        case State::Screen::MAIN:
            update = &Game::updateMain;
            render = &Game::renderMain;
            break;
        case State::Screen::GAME:
            paused = appState == State::App::HOLD;
            update = &Game::updateGame;
            render = &Game::renderGame;
            break;
        default:
            update = &Game::updateUnit;
            render = &Game::renderUnit;
    };
}

void Game::resizeTitle() {

    titleFontSize = window.scale(TITLE_FONT_SIZE);
    bgTitleFontSize = window.scale(TITLE_BG_FONT_SIZE);

    float titleTextSize = MeasureText(title, titleFontSize);
    titleX = window.halfWidthf-titleTextSize*0.45f;
    titleY = window.halfHeightf-titleFontSize*1.5f;

    float bgTitleTextSize = MeasureText(title, bgTitleFontSize);
    bgTitleX = -bgTitleTextSize*0.5f;
    bgTitleY = -bgTitleFontSize;

    titleHintFontSize = window.scale(TITLE_HINT_FONT_SIZE);
    float titleHintTextSize = MeasureText(titleHint, titleHintFontSize);
    titleHintX = window.halfWidth - titleHintTextSize*0.5f;
    titleHintY = static_cast<int>(window.height - window.height*0.25f - titleHintFontSize*0.5f);
}

void Game::resize(int height, int width) {
    // Main and Title screen resize handled in App

}

void Game::unload(){
    UnloadFont(bgTitleFont);
    UnloadFont(titleFont);
}
