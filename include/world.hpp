#pragma once

// #include "config.hpp"
#include "type.hpp"
#include "window.hpp"
#include "grid.hpp"
#include "chip.hpp"

#include "raylib.h"

class World : public Layer {
    Sound splat;
    Window& window;
    Grid grid{window};

    std::vector<Chip> chips;

    int dummyGoalTracker = 0;

public:
    World(Window& window): window(window) {}
    ~World() = default;
    
    void (World::*render)() const = &World::renderUnit;
    WorldState (World::*update)(InputEvent, Action::Surface) = &World::updateUnit;

    void load();
    void spawnChip(Hex::Point hex, int value);
    void createChip(Hex::Point hex, int value);
    void respawnChip(Hex::Point hex, int value);

    void renderUnit() const {};
    void renderMain() const;
    void renderGame() const;
    void renderHold() const;

    WorldState updateUnit(InputEvent, Action::Surface) { return { .reachedGoal = false }; };
    WorldState updateMain(InputEvent, Action::Surface);
    WorldState updateGame(InputEvent, Action::Surface);
    WorldState updateHold(InputEvent, Action::Surface);
    
    void resize(int width, int height) override;
    void transition(State::App, State::Screen);
    void unload();
};
