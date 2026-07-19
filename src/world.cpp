#include "world.hpp"

#include "index.h"
#include "type.hpp"

#include "raylib.h"

void World::load(){
    splat = LoadSound(PATH_ASSET(URI_SOUND_SPLAT));
    grid.load();

    window.enlist(&grid);

    // reserve hex numbers plus shim chip
    chips.reserve(grid.size() + 1);
    // shim chip
    chips.emplace_back(Hex::Point(0, 0, 0), Vector2({}), 0);

    // createChip(Hex::Direction[DIR_UP_L], 6);
    // Hex::State hex = grid.getState(Hex::Direction[DIR_UP_L]);
    // Chip::State chip = chips.at(hex.key).getState();
    createChip(grid.corner(Hex::Unit::UP), 12);
    createChip(grid.corner(Hex::Unit::UP_R), 2);
    createChip(grid.corner(Hex::Unit::DN_R), 4);
    createChip(grid.corner(Hex::Unit::DN), 6);
    createChip(grid.corner(Hex::Unit::DN_L), 8);
    createChip(grid.corner(Hex::Unit::UP_L), 10);
    createChip(Hex::RotateClockwise2[Hex::UP], 8);
    // createChip(Hex::RotateCounterwise1[DIR_UP_L], 5);
    // createChip(Hex::RotateCounterwise2[DIR_UP_L], 4);
    // createChip(Hex::Reverse[Hex::Cardinal::NORTH_WEST], 4);
    // createChip(Hex::Reverse[Hex::Cardinal::NORTH], 6);
    // createChip(Hex::Reverse[Hex::Cardinal::NORTH_EAST], 8);
    // createChip(Hex::Reverse[Hex::Cardinal::SOUTH_EAST], 10);
}

void World::spawnChip(Hex::Point hex, int value) {
    // if (!grid.within(hex)) return TraceLog(LOG_ERROR, "Bad hex for spawning sigil!");

    if (static_cast<int>(chips.size()) > grid.size()) {
        respawnChip(hex, value);
    } else {
        createChip(hex, value);
    }
}

void World::createChip(Hex::Point hex, int value) {
    int total = static_cast<int>(chips.size());
    grid.place(hex, total);
    chips.emplace_back(hex, grid.getPosition(hex), value, true);
}

void World::respawnChip(Hex::Point hex, int value) {
    // WARNING: do not use ANCHOR sigil at 0!
    int total = static_cast<int>(chips.size());
    for (int i = 1; i < total; ++i) {
        Chip& chip = chips.at(i);
        if (chip.available()) {
            chip.enable();

            grid.place(hex, total);
            chip.reset(hex, grid.getPosition(hex), Vector2({ 1.0f, 1.0f }), 0.0f, value);

            break;
        }
    }
}


void World::renderMain() const {
    DrawRectangleGradientV(0, 0, window.width, window.height, DARKBLUE, DARKGRAY);
}

void World::renderGame() const {
    DrawRectangleGradientV(0, 0, window.width, window.height, BLUE, GREEN);
    grid.render();
    
    for (auto& chip : chips) {
        if (chip.active()) {
            chip.render();
        }
    }
}

void World::renderHold() const {
    renderGame();
    const char* pausedText = TextFormat("PAUSED");
    float pausedTextWidth = MeasureText(pausedText, 200);
    DrawText(pausedText, window.halfWidthf-pausedTextWidth*0.5f, window.halfHeightf-100.0f, 200, RAYWHITE);
}

WorldState World::updateMain(InputEvent, Action::Surface){
    return { .reachedGoal = false };
}

WorldState World::updateHold(InputEvent inputEvent, Action::Surface action){
    return { .reachedGoal = false };
}

WorldState World::updateGame(InputEvent inputEvent, Action::Surface action){

    if (inputEvent.id == Event::Input::MOVE_UP || action == Action::Surface::MOVE_UP ) {
            TraceLog(LOG_INFO, "MOVE UP");
            dummyGoalTracker++;
    } else if (inputEvent.id == Event::Input::MOVE_RIGHT || action == Action::Surface::MOVE_RIGHT ) {
            TraceLog(LOG_INFO, "MOVE RIGHT");

    } else if (inputEvent.id == Event::Input::MOVE_DOWN || action == Action::Surface::MOVE_DOWN ) {
            TraceLog(LOG_INFO, "MOVE DOWN");
            dummyGoalTracker--;

    } else if (inputEvent.id == Event::Input::MOVE_LEFT || action == Action::Surface::MOVE_LEFT ) {
            TraceLog(LOG_INFO, "MOVE LEFT");

    }

    if (dummyGoalTracker >= 3) {
        PlaySound(splat);
        dummyGoalTracker = 0;
        return { .reachedGoal = true };
    } else if (dummyGoalTracker < -2) {
        dummyGoalTracker = 0;
        return { .failedGoal = true };
    }

    return { .reachedGoal = false };
}

void World::transition(State::App appState, State::Screen screen) {
    switch(screen) {
        case State::Screen::MAIN:
            update = &World::updateMain;
            render = &World::renderMain;
            break;
        case State::Screen::GAME:
            if (appState == State::App::HOLD) {
                update = &World::updateHold;
                render = &World::renderHold;
            } else {
                update = &World::updateGame;
                render = &World::renderGame;
            }
            break;
        default:
            update = &World::updateUnit;
            render = &World::renderUnit;
    };
}

void World::unload(){
    UnloadSound(splat);
}

void World::resize(int width, int height) {
    TraceLog(LOG_INFO, "HELLO FROM WORLD RESIZE %i %i", width, height);
}