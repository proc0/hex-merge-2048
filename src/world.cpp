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
    chips.emplace_back(Hex::Point(0, 0, 0), Vector2({}), 0, 0);
    // createChip(Hex::Direction[DIR_UP_L], 6);
    // Hex::State hex = grid.getState(Hex::Direction[DIR_UP_L]);
    // Chip::State chip = chips.at(hex.key).getState();
    int key1 = createChip(grid.corner(Hex::Unit::UP), 4);
    grid.place(grid.corner(Hex::Unit::UP), key1);

    int key2 = createChip(grid.corner(Hex::Unit::UP_R), 2);
    grid.place(grid.corner(Hex::Unit::UP_R), key2);

    int key3 = createChip(grid.corner(Hex::Unit::DN_R), 2);
    grid.place(grid.corner(Hex::Unit::DN_R), key3);

    int key4 = createChip(grid.corner(Hex::Unit::UP_L), 4);
    grid.place(grid.corner(Hex::Unit::UP_L), key4);

}

int World::spawnChip(Hex::Point hex, int value) {
    // if (!grid.within(hex)) return TraceLog(LOG_ERROR, "Bad hex for spawning sigil!");
    int key = -1;
    if (static_cast<int>(chips.size()) > grid.size()+1) {
        key = respawnChip(hex, value);
    } else {
        key = createChip(hex, value);
    }

    return key;
}

int World::createChip(Hex::Point hex, int value) {
    int key = static_cast<int>(chips.size());
    grid.place(hex, key);
    chips.emplace_back(hex, grid.getPosition(hex), key, value, true);
    chipsIdxsReady.push_back(key);

    return key;
}

int World::respawnChip(Hex::Point hex, int value) {
    int key = -1;
    // WARNING: do not use ANCHOR sigil at 0!
    int total = static_cast<int>(chips.size());
    for (int i = 1; i < total; ++i) {
        Chip& chip = chips.at(i);
        if (chip.available()) {
            chip.enable();

            grid.place(hex, i);
            chip.place(hex, grid.getPosition(hex), value);
            chipsIdxsReady.push_back(i);
            key = i;
            break;
        }
    }

    return key;
}

void World::updateChip(Hex::Point sourceHex, Hex::Point moveStep) {
    // TODO: make this safety mechanism a member field
    // and just call one function within the while loop that increments
    // and checks an upper bound across all while loops
    int maxTries = 30;

    Hex::Point targetHex = grid.walk(sourceHex, moveStep);
    // TODO: rename isEmpty to grid.vacant? and add a counterpart grid.filled/occupied?
    if (!grid.isEmpty(targetHex)) {
        return;
    }

    Hex::Point lastTarget = targetHex;
    while (maxTries > 0 && !grid.isDirectionEdge(targetHex, moveStep) && grid.isEmpty(targetHex)) {
        lastTarget = targetHex;
        targetHex = grid.walk(targetHex, moveStep);
        maxTries--;
    }

    int sourceKey = grid.getState(sourceHex).key;
    Chip& sourceChip = chips.at(sourceKey);

    if (!grid.isEmpty(targetHex)) {
        int targetKey = grid.getState(targetHex).key;
        Chip& targetChip = chips.at(targetKey);
        // TODO: right an isEqual on Chip
        int targetValue = targetChip.getValue();
        int sourceValue = sourceChip.getValue();
        if (targetValue == sourceValue) {
            grid.clear(sourceHex);
            targetChip.addValue(sourceValue);
            sourceChip.disable();
            // NOTE: might be needed for animations
            // sourceChip.setPosition(grid.getPosition(targetHex));
            // sourceChip.setCurrentHex(targetHex);
            return;
        } else if (grid.isEmpty(lastTarget)) {
            targetHex = lastTarget;
        } else {
            return;
        }
    } 

    grid.clear(sourceHex);
    grid.place(targetHex, sourceKey);
    sourceChip.setPosition(grid.getPosition(targetHex));
    sourceChip.setCurrentHex(targetHex);
}

void World::searchGrid(Hex::Point sourceHex, Hex::Point searchStep, Hex::Point moveStep) {
    int maxTries = 30;
    
    Hex::Point nextHex = grid.walk(sourceHex, searchStep);

    while (maxTries > 0 && !grid.isDirectionEdge(nextHex, searchStep)) {
        // TraceLog(LOG_INFO, "HEX WALK %d %d %d", nextHex.q, nextHex.r, nextHex.s);
        if (!grid.isEmpty(nextHex)){
            updateChip(nextHex, moveStep);
        }
        nextHex = grid.walk(nextHex, searchStep);
        maxTries--;
    }

    // TraceLog(LOG_INFO, "HEX WALK %d %d %d", nextHex.q, nextHex.r, nextHex.s);
    if (!grid.isEmpty(nextHex)){
        updateChip(nextHex, moveStep);
    } 

}

void World::updateMove(Hex::Cardinal dir) {
    Hex::Point moveStep = Hex::Direction[dir];
    // direction of sweep walk through center hex column
    Hex::Point stepBack = Hex::Reverse[dir];
    // TODO: why cant I use [dir]?
    Hex::Cardinal oppositeDir = Hex::Opposite.at(dir);
    // direction of side flank sweeps as we are stepping back
    Hex::Point stepLeft = Hex::RotateClockwise1[oppositeDir];
    Hex::Point stepRight = Hex::RotateCounterwise1[oppositeDir];

    int maxTries = 30;
    // start with the corner hex in the direction of movement
    Hex::Point nextHex = grid.corner(moveStep);
    while (maxTries > 0 && !grid.isDirectionEdge(nextHex, stepBack)) {

        // TraceLog(LOG_INFO, "HEX WALK %d %d %d", nextHex.q, nextHex.r, nextHex.s);
        if (!grid.isEmpty(nextHex)){
            updateChip(nextHex, moveStep);
        }
        searchGrid(nextHex, stepRight, moveStep);
        searchGrid(nextHex, stepLeft, moveStep);

        nextHex = grid.walk(nextHex, stepBack);
        maxTries--;
    }

    // TraceLog(LOG_INFO, "HEX WALK %d %d %d", nextHex.q, nextHex.r, nextHex.s);
    if (!grid.isEmpty(nextHex)){
        updateChip(nextHex, moveStep);
    }
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
    } else if (inputEvent.id == Event::Input::MOVE_UP_RIGHT || action == Action::Surface::MOVE_UP_RIGHT ) {
            TraceLog(LOG_INFO, "MOVE UP RIGHT");
            updateMove(Hex::UP_R);

    } else if (inputEvent.id == Event::Input::MOVE_RIGHT || action == Action::Surface::MOVE_RIGHT ) {
            TraceLog(LOG_INFO, "MOVE RIGHT");
            updateMove(Hex::DN_R);

    } else if (inputEvent.id == Event::Input::MOVE_DOWN || action == Action::Surface::MOVE_DOWN ) {
            TraceLog(LOG_INFO, "MOVE DOWN");
            updateMove(Hex::DN);

    } else if (inputEvent.id == Event::Input::MOVE_LEFT || action == Action::Surface::MOVE_LEFT ) {
            TraceLog(LOG_INFO, "MOVE LEFT");
            updateMove(Hex::DN_L);

    } else if (inputEvent.id == Event::Input::MOVE_UP_LEFT || action == Action::Surface::MOVE_UP_LEFT ) {
            TraceLog(LOG_INFO, "MOVE UP LEFT");
            updateMove(Hex::UP_L);
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
