#include "world.hpp"

#include "index.h"
#include "type.hpp"

#include "raylib.h"

void World::load(){
    splat = LoadSound(PATH_ASSET(URI_SOUND_SPLAT));
    grid.load();

    window.enlist(&grid);

    // reserve hex numbers plus shim chip
    int chipsCapacity = grid.size() + 1;
    chips.reserve(chipsCapacity);
    chipsIdxsReady.reserve(chipsCapacity-1);
    chipsIdxsMoving.reserve(chipsCapacity-1);
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
    chipsIdxsReady.push_back(total);
}

void World::respawnChip(Hex::Point hex, int value) {
    // WARNING: do not use ANCHOR sigil at 0!
    int total = static_cast<int>(chips.size());
    for (int i = 1; i < total; ++i) {
        Chip& chip = chips.at(i);
        if (chip.available()) {
            chip.enable();

            grid.place(hex, i);
            chip.place(hex, grid.getPosition(hex), value);
            chipsIdxsReady.push_back(i);
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

void World::updateChip(Hex::Point source, Hex::Point step) {
    // int chipKey = grid.getState(source).key;
    // Chip& chip = chips.at(chipKey);
    // Vector2 chipPos = chip.getPosition();
    // TraceLog(LOG_INFO, "CHIP %d (%d) at %f %f", chipKey, chip.getValue(), chipPos.x, chipPos.y);
}

void World::searchGrid(Hex::Point sourceHex, Hex::Point hexStep) {
    int maxTries = 30;
    
    Hex::Point nextHex = grid.walk(sourceHex, hexStep);
    while (maxTries > 0 && !grid.isDirectionEdge(nextHex, hexStep)) {
        TraceLog(LOG_INFO, "HEX WALK %d %d %d", nextHex.q, nextHex.r, nextHex.s);

        nextHex = grid.walk(nextHex, hexStep);
        maxTries--;
    }
    
    TraceLog(LOG_INFO, "HEX WALK %d %d %d", nextHex.q, nextHex.r, nextHex.s);

}

void World::updateMove(Hex::Cardinal dir) {
    Hex::Point step = Hex::Direction[dir];
    // direction of sweep walk through center hex column
    Hex::Point stepBack = Hex::Reverse[dir];
    // TODO: why cant I use [dir]?
    Hex::Cardinal oppositeDir = Hex::Opposite.at(dir);
    // direction of side flank sweeps as we are stepping back
    Hex::Point stepLeft = Hex::RotateClockwise1[oppositeDir];
    Hex::Point stepRight = Hex::RotateCounterwise1[oppositeDir];

    int maxTries = 30;
    // start with the corner hex in the direction of movement
    Hex::Point nextHex = grid.corner(step);
    while (maxTries > 0 && !grid.isDirectionEdge(nextHex, stepBack)) {

        TraceLog(LOG_INFO, "HEX WALK %d %d %d", nextHex.q, nextHex.r, nextHex.s);
        // if (!grid.isEmpty(nextHex)){
        //     updateChip(nextHex, step);
        // }
        searchGrid(nextHex, stepRight);
        searchGrid(nextHex, stepLeft);
        // Hex::Point nextHexRight = grid.walk(nextHex, stepRight);
        // while (maxTries > 0 && !grid.isDirectionEdge(nextHexRight, stepRight)) {
        //     TraceLog(LOG_INFO, "HEX WALK RIGHT %d %d %d", nextHexRight.q, nextHexRight.r, nextHexRight.s);
        //     // if (!grid.isEmpty(nextHexRight)){
        //     //     updateChip(nextHexRight, step);
        //     // }
        //     nextHexRight = grid.walk(nextHexRight, stepRight);
        //     maxTries--;
        // }
        // TraceLog(LOG_INFO, "HEX WALK RIGHT %d %d %d", nextHexRight.q, nextHexRight.r, nextHexRight.s);

        // Hex::Point nextHexLeft = grid.walk(nextHex, stepLeft);
        // while (maxTries > 0 && !grid.isDirectionEdge(nextHexLeft, stepLeft)) {
        //     TraceLog(LOG_INFO, "HEX WALK LEFT %d %d %d", nextHexLeft.q, nextHexLeft.r, nextHexLeft.s);
        //     // if (!grid.isEmpty(nextHexLeft)){
        //     //     updateChip(nextHexLeft, step);
        //     // }
        //     nextHexLeft = grid.walk(nextHexLeft, stepLeft);
        //     maxTries--;
        // }
        // TraceLog(LOG_INFO, "HEX WALK LEFT %d %d %d", nextHexLeft.q, nextHexLeft.r, nextHexLeft.s);

        nextHex = grid.walk(nextHex, stepBack);
        maxTries--;
    }

    TraceLog(LOG_INFO, "HEX WALK %d %d %d", nextHex.q, nextHex.r, nextHex.s);

    // std::erase_if(chipsIdxsReady, [this, dir](int idx){
    //     Chip& chip = chips[idx];
    //     Vector2 chipPos = chip.getPosition();
    //     TraceLog(LOG_INFO, "READY CHIP %d (%d) at %f %f", idx, chip.getValue(), chipPos.x, chipPos.y);
    //     chipsIdxsMoving.push_back(idx);
    //     return dir == Hex::UP;
    // });
    // std::erase_if(chipsIdxsMoving, [this, dir](int idx){
    //     Chip& chip = chips[idx];
    //     // Vector2 chipPos = chip.getPosition();
    //     TraceLog(LOG_INFO, "MOVING CHIP %d (%d)", idx, chip.getValue());

    //     return dir == Hex::DN;
    // });
    
    TraceLog(LOG_INFO, "----------- END TURN -----------");
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
            updateMove(Hex::UP);
    } else if (inputEvent.id == Event::Input::MOVE_RIGHT || action == Action::Surface::MOVE_RIGHT ) {
            TraceLog(LOG_INFO, "MOVE RIGHT");

    } else if (inputEvent.id == Event::Input::MOVE_DOWN || action == Action::Surface::MOVE_DOWN ) {
            TraceLog(LOG_INFO, "MOVE DOWN");
            updateMove(Hex::DN);

    } else if (inputEvent.id == Event::Input::MOVE_LEFT || action == Action::Surface::MOVE_LEFT ) {
            TraceLog(LOG_INFO, "MOVE LEFT");

    }

    // if (dummyGoalTracker >= 3) {
    //     PlaySound(splat);
    //     dummyGoalTracker = 0;
    //     return { .reachedGoal = true };
    // } else if (dummyGoalTracker < -2) {
    //     dummyGoalTracker = 0;
    //     return { .failedGoal = true };
    // }

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
    Vector2 gridUnit = grid.getUnit();
    for (auto& chip : chips) {
        if (chip.active()) {
            Vector2 newPosition = grid.getPosition(chip.getCurrentHex());
            chip.setPosition(newPosition);
        }
        // TODO: make consistent grid.unit (Vector2) and chip.size + chip.scale
        // should it be size*scale or a Vector2 size? Should be the same in both.
        chip.setSize(gridUnit.x);
    }
}
