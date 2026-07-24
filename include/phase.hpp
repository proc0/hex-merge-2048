#pragma once

#include "type.hpp"
#include "level.hpp"

class Phase {
	
	int currentPhase = 0;

	const Distribution* valueDist = &distributionEasy;

public:
	Phase() {}
	~Phase() = default;

	void load();

	void update();
	void setPhase(int value);
    
    int getRandomValue() const;
	
	void transition(Action::Surface);
};
