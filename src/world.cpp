#include "world.hpp"

#include "index.h"
#include "config.hpp"
#include "hex.hpp"
#include "type.hpp"
#include "level.hpp"

#include "raylib.h"

// #include <string>

void World::load(){
    splat = LoadSound(PATH_ASSET(URI_SOUND_SPLAT));

    grid.load();
    window.enlist(&grid);

    // reserve number of hexes plus shim chip
    int chipsCapacity = grid.size() + 1;
    chips.reserve(chipsCapacity);
    // TODO: rename
    chipsIdxsMoving.reserve(chipsCapacity);
    chipsIdxsUpdating.reserve(chipsCapacity);

    // shim chip
    chips.emplace_back(Hex::Origin, Vector2({}), 0, 0);

    // for (int i = 0; i < randomizedPhaseMap.size(); ++i) {
    //     std::string tempStr = "";
    //     auto& randMap = randomizedPhaseMap[i];
    //     for (int j = 0; j < randMap.size(); ++j) {
    //         tempStr = std::format("{} {}", tempStr, randMap[j]);
    //     }
    //     TraceLog(LOG_INFO, "%d: %s", i, tempStr.c_str());
    // }
}

void World::reset() {
    grid.reset();
    for (auto& chip : chips) {
        chip.reset();
    }
    meta.maxValue = 2;
    meta.gridlock = false;
    chipsIdxsUpdating.clear();
    chipsIdxsMoving.clear();
    // seed chip
    grid.place(Hex::Origin, spawnChip(Hex::Origin, 2));
    meta.state = State::World::WAIT;
}

int World::spawnChip(Hex::Point hex, int value) {
    int key = 0;
    if (static_cast<int>(chips.size()) > grid.size()) {
        key = respawnChip(hex, value);
    } else {
        key = createChip(hex, value);
    }

    return key;
}

int World::createChip(Hex::Point hex, int value) {
    int key = static_cast<int>(chips.size());
    // update grid hex with key
    grid.place(hex, key);
    // get current hex size and scale font
    Vector2 unitHex = grid.getUnit();
    int fontSize = static_cast<int>(window.scale(CHIP_FONT_SIZE));
    // create and resize chip
    chips.emplace_back(hex, grid.getPosition(hex), key, value, true);
    chips.at(key).resize(unitHex, fontSize);

    chipsIdxsUpdating.push_back(key);
    return key;
}

int World::respawnChip(Hex::Point hex, int value) {
    int key = -1;
    // WARNING: do not use SHIM sigil at 0!
    int total = static_cast<int>(chips.size());
    for (int i = 1; i < total; ++i) {
        Chip& chip = chips.at(i);
        if (chip.available()) {
            chip.enable();

            grid.place(hex, i);
            chip.place(hex, grid.getPosition(hex), value);
            key = i;

            chipsIdxsUpdating.push_back(key);
            break;
        }
    }

    return key;
}

void World::updateChip(Hex::Basis forward, Hex::Point sourceHex) {
    // source hex has a chip, check one hex forward
    Hex::Point targetHex = grid.walk(forward, sourceHex);
    // grid.walk will return source hex if no movement possible
    // if no movement possible for chip, end walk
    if (targetHex == sourceHex) {
        return;
    }
    // start walking forward in the direction of player input
    // save a backup slot for backtracking if target is occupied
    Hex::Point lastTargetHex = targetHex;
    while (!grid.walkEdge(forward, targetHex) && grid.vacant(targetHex)) {
        if (arrested()) break;

        lastTargetHex = targetHex;
        targetHex = grid.walk(forward, targetHex);
    }

    // target hex is either the edge or occupied
    int sourceKey = grid.getState(sourceHex).key;
    Chip& sourceChip = chips.at(sourceKey);
    // if occupied, check if chips can merge
    if (grid.occupied(targetHex)) {
        int targetKey = grid.getState(targetHex).key;
        Chip& targetChip = chips.at(targetKey);
        // merge chips
        if (targetChip == sourceChip && !targetChip.hasAbsorbed()) {
            // TraceLog(LOG_INFO, "MERGING %d and %d", targetKey, sourceKey);
            grid.clear(sourceHex);
            int resultValue = targetChip.merge(sourceChip);
            // World book keeping
            if (resultValue > meta.maxValue) {
                meta.maxValue = resultValue;
            }
            chipsIdxsMoving.push_back(sourceKey);
            meta.state = State::World::PROCESS_MOVES;

            return;
        } else if (grid.vacant(lastTargetHex)) {
            // backtrack to last empty hex
            // WARNING: this must continue to after conditional!
            targetHex = lastTargetHex;
        } else {
            // WARNING: this return must be in the else clause
            // of this conditional to allow above conditional
            // to continue and reuse the chip to empty hex move
            return;
        }
    } 

    // move chip to the next empty hex
    // target hex can be either original targetHex
    // from forward search, or the second to last (lastTargetHex)
    grid.clear(sourceHex);
    grid.place(targetHex, sourceKey);
    // update screen position and hex reference on chip
    sourceChip.translate(targetHex, grid.getPosition(targetHex));
    // World book keeping
    chipsIdxsMoving.push_back(sourceKey);
    meta.state = State::World::PROCESS_MOVES;
}

void World::searchGrid(Hex::Basis forward, Hex::Basis sideward, Hex::Point midHex) {
    // check both left and right wings of the current hex column
    Hex::Point sideHex = grid.walk(sideward, midHex);
    while (!grid.walkEdge(sideward, sideHex)) {
        if (arrested()) break;
        // TraceLog(LOG_INFO, "SIDE HEX WALK %d %d %d", sideHex.q, sideHex.r, sideHex.s);
        if (grid.occupied(sideHex)){
            updateChip(forward, sideHex);
        }
        sideHex = grid.walk(sideward, sideHex);
    }
    // TraceLog(LOG_INFO, "END SIDE HEX WALK %d %d %d", sideHex.q, sideHex.r, sideHex.s);
    if (grid.occupied(sideHex)){
        updateChip(forward, sideHex);
    } 
}

void World::updateMove(Hex::Cardinal needle) {
    // direction of movement of chips and player input
    Hex::Basis forward = Hex::BasisDirection[needle];
    // direction of sweep walk through center hex column
    Hex::Basis backward = Hex::BasisReversed[needle];
    // direction of side flank sweeps as we are stepping back
    Hex::Basis leftward = Hex::BasisRotatedLeft2[needle];
    Hex::Basis rightward = Hex::BasisRotatedRight2[needle];

    arrest(66);
    // start with the corner hex in the direction of movement
    Hex::Point midHex = grid.corner(forward);
    while (!grid.walkEdge(backward, midHex)) {
        if (arrested()) break;
        // TraceLog(LOG_INFO, "MID HEX WALK %d %d %d", midHex.q, midHex.r, midHex.s);
        if (grid.occupied(midHex)){
            updateChip(forward, midHex);
        }
        // walk the grid sideways
        searchGrid(forward, rightward, midHex);
        searchGrid(forward, leftward, midHex);
        // walk the grid backward
        midHex = grid.walk(backward, midHex);
    }
    // TraceLog(LOG_INFO, "END HEX WALK %d %d %d", midHex.q, midHex.r, midHex.s);
    if (grid.occupied(midHex)){
        updateChip(forward, midHex);
    }

    // TraceLog(LOG_INFO, "----------- END TURN -----------");
}

void World::renderHold() const {
    renderGame();
}

WorldState World::updateMain(InputEvent, Action::Surface){
    return meta;
}

WorldState World::updateHold(InputEvent inputEvent, Action::Surface action){
    return meta;
}

WorldState World::updateGame(InputEvent inputEvent, Action::Surface action){

    if (chipsIdxsUpdating.size() && meta.state == State::World::PROCESS_SPAWN) {
        // TraceLog(LOG_INFO, "Updating non-movement chips.");
        std::erase_if(chipsIdxsUpdating, [this](int idx){
            Chip& chip = chips[idx];
            State::Chip chipState = chip.update();
            return chipState == State::Chip::READY;
        });

        if (chipsIdxsUpdating.empty()) {
            meta.state = State::World::WAIT;
        }
    }

    if (chipsIdxsMoving.size() && meta.state == State::World::PROCESS_MOVES) {
        // TraceLog(LOG_INFO, "Processing movement chips.");
        std::erase_if(chipsIdxsMoving, [this](int idx){
            Chip& chip = chips[idx];
            State::Chip chipState = chip.update();

            return chipState == State::Chip::READY;
        });


        if (chipsIdxsMoving.empty()) {
            meta.state = State::World::PROCESS_SPAWN;
            // TraceLog(LOG_INFO, "DONE Processing movement chips. Size: %d", chipsIdxsMoving.size());

            for (auto& chip : chips) {
                if (chip.active()) {
                    chip.sync();
                }
            }

            chipsIdxsUpdating.clear();

            // TODO: separate spawning into its own method
            // potentially have another config for number of chips that spawn
            // like using the log of maxValue
            int numberOfChips = 1;
            if (meta.maxValue > 32) {
                numberOfChips = 2;
            } else if (meta.maxValue > 512) {
                numberOfChips = 3;
            }
            for (int i = 0; i < numberOfChips; i++) {                
                Hex::Point nextHex = grid.findRandom();
                if (nextHex != Hex::Absurd) {
                    spawnChip(nextHex, getRandomValue());
                }
            }

            // check gridlock
            if (grid.filled()) {
                bool gridlock = true;
                for (auto& chip : chips) {
                    if (chip.active() && !chipLocked(chip)) {
                        gridlock = false;
                        break;
                    }
                }

                if (gridlock) {
                    meta.state = State::World::LOCKED;
                    meta.gridlock = true;
                }
            }
        }
    }

    // NOTE: remove PROCESS_SPAWN state check to make it wait for all animations and make it feel "turn-based"
    // if (meta.state == State::World::WAIT) {        
    if (meta.state == State::World::WAIT || meta.state == State::World::PROCESS_SPAWN) {        
        if (inputEvent.id == Event::Input::MOVE_UP || action == Action::Surface::MOVE_UP ) {
                // TraceLog(LOG_INFO, "MOVE UP");
                updateMove(Hex::N);
        } else if (inputEvent.id == Event::Input::MOVE_UP_RIGHT || action == Action::Surface::MOVE_UP_RIGHT ) {
                // TraceLog(LOG_INFO, "MOVE UP RIGHT");
                updateMove(Hex::NE);

        } else if (inputEvent.id == Event::Input::MOVE_RIGHT || action == Action::Surface::MOVE_RIGHT ) {
                // TraceLog(LOG_INFO, "MOVE RIGHT");
                updateMove(Hex::SE);

        } else if (inputEvent.id == Event::Input::MOVE_DOWN || action == Action::Surface::MOVE_DOWN ) {
                // TraceLog(LOG_INFO, "MOVE DOWN");
                updateMove(Hex::S);

        } else if (inputEvent.id == Event::Input::MOVE_LEFT || action == Action::Surface::MOVE_LEFT ) {
                // TraceLog(LOG_INFO, "MOVE LEFT");
                updateMove(Hex::SW);

        } else if (inputEvent.id == Event::Input::MOVE_UP_LEFT || action == Action::Surface::MOVE_UP_LEFT ) {
                // TraceLog(LOG_INFO, "MOVE UP LEFT");
                updateMove(Hex::NW);
        }
    }

    return meta;
}

bool World::chipLocked(Chip& chip) const {
    bool locked = true;

    Hex::Point chipHex = chip.getHex();
    for (auto& direction : Hex::BasisDirection) {
        Hex::Point neighbor = chipHex + direction;
        if (grid.inside(neighbor)) {
            if (grid.vacant(neighbor)) {
                locked = false;
                break;
            }

            int key = grid.getState(neighbor).key;
            const Chip& nextChip = chips.at(key);
            bool canMerge = nextChip.getValue() == chip.getValue();
            if (canMerge) {
                locked = false;
                break;
            }
        }
    }

    return locked;
}

int World::getRandomValue() const {
    // get the phase index
    // TODO: separate into a different method to get phase index from other places
    // subtract min chip value (2) and take the min between the value and max chip value (1024)
    int currentPhase = static_cast<int>(fmin(fmax(log2(meta.maxValue), 0), PHASE_COUNT))-1;
    
    if (currentPhase > PHASE_COUNT-1) {
        // clamp the phase to the last one
        currentPhase = PHASE_COUNT-1;
    }
    
    int newValue = 2;
    if (currentPhase >= 0) {
        // sample the random distribution with a random index
        int randomIndex = GetRandomValue(0, DISTRIBUTION_RESOLUTION-1);
        newValue = randomizedPhaseMap[currentPhase][randomIndex];
    }

    return fmax(newValue, 2);
}

void World::renderMain() const {
    DrawRectangleGradientV(0, 0, window.width, window.height, DARKBLUE, DARKGRAY);
}

void World::renderGame() const {
    DrawRectangleGradientV(0, 0, window.width, window.height, BLUE, GREEN);
    grid.render();
    
    for (auto& chip : chips) {
        if (chip.active() && chip.hasAbsorbed()) {
            chip.render();
        }
    }

    for (auto& chip : chips) {
        if (chip.active() && !chip.hasAbsorbed()) {
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

bool World::arrested() {
    attempts--;
    return attempts < 0;
}

void World::arrest(int max) {
    attempts = max;
}

void World::resize(int width, int height) {
    Vector2 unitHex = grid.getUnit();
    int fontSize = static_cast<int>(window.scale(CHIP_FONT_SIZE));
    for (auto& chip : chips) {
        chip.resize(unitHex, fontSize);
        if (chip.active()) {
            Vector2 newPosition = grid.getPosition(chip.getHex());
            chip.setPosition(newPosition);
        }
    }
}

void World::unload(){
    for (auto& chip : chips) {
        chip.unload();
    }
    chips.clear();
    grid.unload();
    UnloadSound(splat);
}
