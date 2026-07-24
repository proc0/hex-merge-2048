#include "game.hpp"

#include "type.hpp"

#include "raylib.h"

void Game::load() {
    updateTitle();
}

void Game::start() {
    meta.state = State::Game::PLAY;
    gameTimerId = window.timer.startWatch();
    meta.totalTimeId = gameTimerId;
}

void Game::restart() {
    meta.score = 0;
    meta.state = State::Game::PLAY;
    gameTimerId = window.timer.startWatch();
    meta.totalTimeId = gameTimerId;
}

void Game::continueGame() {
    meta.state = State::Game::STAY;
}

void Game::renderMain() const {
    DrawText(title, titleX, titleY, titleFontSize, RAYWHITE);
}

void Game::renderGame() const {

}

void Game::renderTitle() const {
    DrawRectangleGradientV(0, 0, window.width, window.height, DARKBLUE, ORANGE);
    DrawText(title, titleX, titleY, titleFontSize, RAYWHITE);
    DrawText(titleHint, titleHintX, titleHintY, titleHintFontSize, RAYWHITE);
}

GameState Game::updateMain(InputEvent, WorldState){
    return meta;
}

GameState Game::updateGame(InputEvent inputEvent, WorldState worldState){
    if (meta.state != State::Game::PLAY && meta.state != State::Game::STAY) return meta;
    
    if (paused) {
        meta.state = State::Game::PAUSE;
        return meta;
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
    float titleTextSize = MeasureText(title, titleFontSize);
    titleX = window.halfWidth - titleTextSize*0.5f;
    titleY = window.halfHeight - titleFontSize;

    float titleHintTextSize = MeasureText(titleHint, titleHintFontSize);
    titleHintX = window.halfWidth - titleHintTextSize*0.5f;
    titleHintY = static_cast<int>(window.height - window.height*0.25f - titleHintFontSize*0.5f);
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

void Game::resize(int height, int width) {

}

void Game::unload(){

}
