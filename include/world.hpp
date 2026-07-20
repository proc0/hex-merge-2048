#pragma once

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
    std::vector<int> chipsIdxsMoving;

    int attempts;

    WorldState meta;

public:
    World(Window& window): window(window) {}
    ~World() = default;
    
    void (World::*render)() const = &World::renderUnit;
    WorldState (World::*update)(InputEvent, Action::Surface) = &World::updateUnit;

    void load();
    int spawnChip(Hex::Point hex, int value);
    int createChip(Hex::Point hex, int value);
    int respawnChip(Hex::Point hex, int value);

    void renderUnit() const {};
    void renderMain() const;
    void renderGame() const;
    void renderHold() const;

    void updateMove(Hex::Cardinal);
    void updateChip(Hex::Basis moveStep, Hex::Point source);
    void searchGrid(Hex::Basis moveStep, Hex::Basis searchStep, Hex::Point source);
    WorldState updateUnit(InputEvent, Action::Surface) { return defaultWorldState; };
    WorldState updateMain(InputEvent, Action::Surface);
    WorldState updateGame(InputEvent, Action::Surface);
    WorldState updateHold(InputEvent, Action::Surface);
    
    bool chipLocked(Chip&) const;
    int getRandomValue() const;
    void resize(int width, int height) override;
    void transition(State::App, State::Screen);
    bool arrested();
    void arrest(int max);
    void unload();
};
