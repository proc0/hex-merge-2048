#pragma once

#include "type.hpp"
#include "level.hpp"

#include "raylib.h"

#include <span>

class Phase {
	
	std::span<const ValueDistribution, PHASE_COUNT> valueDist = distributionEasy;
	std::span<const int, PHASE_COUNT> spawnDist = spawnNumberPhasesEasy;

	// TODO: have a dedicated mode enum type and then map to action?
	Action::Surface mode = Action::Surface::MAIN_NEW_CLASSIC;

	int currentPhase = 0;

public:
	Phase() {}
	~Phase() = default;

	void load();

	void update();
	void setPhase(int value);
    
    int getPhase() const;
    int getMode() const;
    Color getPhaseColor() const;
    int getRandomValue() const;
	int getSpawnNumber() const;

	void transition(Action::Surface);
};
