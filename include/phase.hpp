#pragma once

#include "type.hpp"
#include "level.hpp"

#include <span>

class Phase {
	
	int currentPhase = 0;

	// const Distribution* valueDist = &distributionEasy;

	std::span<const ValueDistribution, PHASE_COUNT> valueDist = distributionEasy;
	std::span<const int, PHASE_COUNT> spawnDist = spawnNumberPhasesEasy;
	
public:
	Phase() {}
	~Phase() = default;

	void load();

	void update();
	void setPhase(int value);
    
    int getRandomValue() const;
	int getSpawnNumber() const;

	void transition(Action::Surface);
};
