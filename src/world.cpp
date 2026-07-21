#include "world.hpp"

#include "hex.hpp"
#include "index.h"
#include "type.hpp"

#include "raylib.h"

void World::load(){
    splat = LoadSound(PATH_ASSET(URI_SOUND_SPLAT));

    grid.load();
    window.enlist(&grid);

    // reserve number of hexes plus shim chip
    int chipsCapacity = grid.size() + 1;
    chips.reserve(chipsCapacity);
    // TODO: rename
    chipsIdxsMoving.reserve(chipsCapacity);

    // shim chip
    chips.emplace_back(Hex::Origin, Vector2({}), 0, 0);

    // seed chip
    grid.place(Hex::Origin, createChip(Hex::Origin, 2));
}

void World::reset() {
    grid.reset();
    for (auto& chip : chips) {
        chip.reset();
    }
    // seed chip
    grid.place(Hex::Origin, createChip(Hex::Origin, 2));
    meta.maxValue = 2;
    meta.gridlock = false;
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
    grid.place(hex, key);
    chips.emplace_back(hex, grid.getPosition(hex), key, value, true);

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
            meta.state = State::World::PROCESS;

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
    sourceChip.move(targetHex, grid.getPosition(targetHex));
    // World book keeping
    chipsIdxsMoving.push_back(sourceKey);
    meta.state = State::World::PROCESS;
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
    // const char* pausedText = TextFormat("PAUSED");
    // float pausedTextWidth = MeasureText(pausedText, 200);
    // DrawText(pausedText, window.halfWidthf-pausedTextWidth*0.5f, window.halfHeightf-100.0f, 200, RAYWHITE);
}

WorldState World::updateMain(InputEvent, Action::Surface){
    return meta;
}

WorldState World::updateHold(InputEvent inputEvent, Action::Surface action){
    return meta;
}

WorldState World::updateGame(InputEvent inputEvent, Action::Surface action){

    if (meta.state == State::World::PROCESS) {
        std::erase_if(chipsIdxsMoving, [this](int idx){
            Chip& chip = chips[idx];
            State::Chip chipState = chip.update();

            return chipState == State::Chip::READY;
        });

        if (chipsIdxsMoving.empty()) {
            meta.state = State::World::WAIT;
            for (auto& chip : chips) {
                chip.sync();
            }

            for (int i = 0; i < 4; i++) {                
                Hex::Point nextHex = grid.findRandom();
                if (nextHex != Hex::Absurd) {
                    spawnChip(nextHex, getRandomValue());
                }
            }

            // check gridlock
            if (grid.filled()) {
                bool gridlock = true;
                for (auto& chip : chips) {
                    if (!chipLocked(chip)) {
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

    if (meta.state == State::World::WAIT) {        
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

    Hex::Point chipHex = chip.getCurrentHex();
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
    int nextValue = 2;
    int chance = 0;
    switch (meta.maxValue) {
    case 32:
        chance = GetRandomValue(0, 4);
        if (chance == 4) {
            nextValue = 4;
        } else {
            nextValue = 2;
        }
        break;
    case 128:
        chance = GetRandomValue(0, 6);
        if (chance == 6) {
            nextValue = 8;
        }
        else if (chance == 3 || chance == 4 || chance == 5) {
            nextValue = 4;
        }
        else {
            nextValue = 2;
        }
        break;
    case 512:
        chance = GetRandomValue(0, 6);
        if (chance == 6) {
            nextValue = 16;
        } else if (chance == 5 || chance == 4) {
            nextValue = 8;
        } else if (chance == 3 || chance == 2) {
            nextValue = 4;
        } else {
            nextValue = 2;
        }
        break;
    default:
        nextValue = 2;
    }

    return nextValue;
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

void World::unload(){
    grid.unload();
    UnloadSound(splat);
}
