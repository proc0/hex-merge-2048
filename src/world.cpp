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
    int key1 = createChip(grid.corner(Hex::Basis::W()), 2);
    grid.place(grid.corner(Hex::Basis::W()), key1);

    int key2 = createChip(Hex::Basis::W(), 2);
    grid.place(Hex::Basis::W(), key2);

    int key3 = createChip(Hex::Point(0, 0, 0), 2);
    grid.place(Hex::Point(0, 0, 0), key3);

    int key4 = createChip(Hex::Basis::S(), 2);
    grid.place(Hex::Basis::S(), key4);

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

void World::updateChip(Hex::Basis forward, Hex::Point sourceHex) {
    // source hex has a chip, check one hex forward
    Hex::Point targetHex = grid.walk(forward, sourceHex);
    // no movement possible for chip
    if (grid.occupied(targetHex)) return;
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
        if (targetChip == sourceChip) {
            grid.clear(sourceHex);
            targetChip.merge(sourceChip);
            chipsIdxsMoving.push_back(sourceKey);
            state = State::World::PROCESS;
            // NOTE: needs to be disabled after animation
            // sourceChip.disable();
            // NOTE: might be needed for animations
            // sourceChip.setPosition(grid.getPosition(targetHex));
            // sourceChip.setCurrentHex(targetHex);
            return;
        } else if (grid.vacant(lastTargetHex)) {
            // backtrack to last empty hex
            targetHex = lastTargetHex;
        } else {
            return;
        }
    } 
    // move chip to the next empty hex
    grid.clear(sourceHex);
    grid.place(targetHex, sourceKey);
    // update screen position and hex reference on chip
    sourceChip.move(targetHex, grid.getPosition(targetHex));
    chipsIdxsMoving.push_back(sourceKey);
    state = State::World::PROCESS;
    // sourceChip.setPosition(grid.getPosition(targetHex));
    // sourceChip.setCurrentHex(targetHex);
}

void World::searchGrid(Hex::Basis forward, Hex::Basis sideward, Hex::Point midHex) {
    // check both left and right wings of the current hex column
    Hex::Point sideHex = grid.walk(sideward, midHex);
    while (!grid.walkEdge(sideward, sideHex)) {
        if (arrested()) break;
        // TraceLog(LOG_INFO, "HEX WALK %d %d %d", sideHex.q, sideHex.r, sideHex.s);
        if (grid.occupied(sideHex)){
            updateChip(forward, sideHex);
        }
        sideHex = grid.walk(sideward, sideHex);
    }
    // TraceLog(LOG_INFO, "HEX WALK %d %d %d", sideHex.q, sideHex.r, sideHex.s);
    if (grid.occupied(sideHex)){
        updateChip(forward, sideHex);
    } 
}

void World::updateMove(Hex::Cardinal needle) {
    // direction of movement of chips and player input
    Hex::Basis forward = Hex::Direction[needle];
    // direction of sweep walk through center hex column
    Hex::Basis backward = Hex::Reverse[needle];
    // direction of side flank sweeps as we are stepping back
    Hex::Basis leftward = Hex::RotateCounterwise2[needle];
    Hex::Basis rightward = Hex::RotateClockwise2[needle];

    arrest(66);
    // start with the corner hex in the direction of movement
    Hex::Point midHex = grid.corner(forward);
    while (!grid.walkEdge(backward, midHex)) {
        if (arrested()) break;
        // TraceLog(LOG_INFO, "HEX WALK %d %d %d", midHex.q, midHex.r, midHex.s);
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

    if (state == State::World::PROCESS) {
        state = State::World::WAIT;
        std::erase_if(chipsIdxsMoving, [this](int idx){
            Chip& chip = chips[idx];
            // Vector2 chipPos = chip.getPosition();
            State::Chip chipState = chip.update();
            if (chipState == State::Chip::MOVING) {
                // TraceLog(LOG_INFO, "MOVING CHIP %d (%d)", idx, chip.getValue());
                state = State::World::PROCESS;
            }

            return chipState == State::Chip::READY;
        });
        // for (auto& chip : chips) {
        //     chip.update();
        // }
    } else if (state == State::World::WAIT) {        
        if (inputEvent.id == Event::Input::MOVE_UP || action == Action::Surface::MOVE_UP ) {
                TraceLog(LOG_INFO, "MOVE UP");
                updateMove(Hex::N);
        } else if (inputEvent.id == Event::Input::MOVE_UP_RIGHT || action == Action::Surface::MOVE_UP_RIGHT ) {
                TraceLog(LOG_INFO, "MOVE UP RIGHT");
                updateMove(Hex::NE);

        } else if (inputEvent.id == Event::Input::MOVE_RIGHT || action == Action::Surface::MOVE_RIGHT ) {
                TraceLog(LOG_INFO, "MOVE RIGHT");
                updateMove(Hex::SE);

        } else if (inputEvent.id == Event::Input::MOVE_DOWN || action == Action::Surface::MOVE_DOWN ) {
                TraceLog(LOG_INFO, "MOVE DOWN");
                updateMove(Hex::S);

        } else if (inputEvent.id == Event::Input::MOVE_LEFT || action == Action::Surface::MOVE_LEFT ) {
                TraceLog(LOG_INFO, "MOVE LEFT");
                updateMove(Hex::SW);

        } else if (inputEvent.id == Event::Input::MOVE_UP_LEFT || action == Action::Surface::MOVE_UP_LEFT ) {
                TraceLog(LOG_INFO, "MOVE UP LEFT");
                updateMove(Hex::NW);
        }
    }

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
    UnloadSound(splat);
}