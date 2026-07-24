#pragma once

class Phase {

public:
	Phase() {}
	~Phase() = default;

	void load();

	void update();

	void transition();
};
