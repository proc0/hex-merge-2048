#include "phase.hpp"

#include "tool.hpp"
#include "level.hpp"

#include "raylib.h"
#include "type.hpp"

// #include <string>

void Phase::load() {
	
    // TODO: move to a debug header?
    // debug print distributions
    // for (int i = 0; i < randomizedPhaseMap.size(); ++i) {
    //     std::string tempStr = "";
    //     auto& randMap = randomizedPhaseMap[i];
    //     for (int j = 0; j < randMap.size(); ++j) {
    //         tempStr = std::format("{} {}", tempStr, randMap[j]);
    //     }
    //     TraceLog(LOG_INFO, "%d: %s", i, tempStr.c_str());
    // }
}

void Phase::update() {
	
}

int Phase::getRandomValue() const {

    int newValue = 2;
    if (currentPhase >= 0) {
        // sample the random distribution with a random index
        int randomIndex = GetRandomValue(0, DISTRIBUTION_RESOLUTION-1);
        newValue = valueDist[currentPhase][randomIndex];
    }

    return fmax(newValue, 2);
}

int Phase::getSpawnNumber() const {
	return spawnDist[currentPhase];
}

void Phase::setPhase(int value) {
    // get the phase index
	currentPhase = static_cast<int>(fmin(fmax(log2(value), 0), PHASE_COUNT))-1;
    
    if (currentPhase > PHASE_COUNT-1) {
        // clamp the phase to the last one
        currentPhase = PHASE_COUNT-1;
    } else if (currentPhase < 0) {
    	currentPhase = 0;
    }
}

void Phase::transition(Action::Surface action) {
	switch (action) {
	case Action::Surface::MAIN_NEW_CLASSIC:
		valueDist = distributionEasy;
		spawnDist = spawnNumberPhasesEasy;
		break;
	case Action::Surface::MAIN_NEW_WIPEOUT:
		valueDist = distributionMedium;
		spawnDist = spawnNumberPhasesMedium;
		break;
	default:
		break;
	}
}
