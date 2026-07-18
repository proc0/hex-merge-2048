#pragma once

#include "config.hpp"
#include "type.hpp"

#include "raylib.h"

#include <array>

// WARNING: must equal the properties enum size
#define PROPS_SIZE 10

class Chip {
	std::array<float, PROPS_SIZE> source{0};
	std::array<float, PROPS_SIZE> target{0};
	std::array<float, PROPS_SIZE> current{0};
	std::array<int, PROPS_SIZE> frame{0};

	Hex::Point currentHex;
	Hex::Point targetHex;

	Color primaryColor = LIGHTGRAY;
	Color secondaryColor = RAYWHITE;

	ChipState state;

	int size = HEX_SIZE;
	bool enabled;

	enum {
		POSX,
		POSY,
		SCALEX,
		SCALEY,
		SIZE,
		ROT,
		COL1,
		COL2,
		COL3,
		COL4
	};

public:
	Chip(Hex::Point target, Vector2 position, int value, bool active = false): 
		currentHex(target), 
		targetHex(target), 
		state({ value }),
		enabled(active) {
    		TraceLog(LOG_INFO, "CREATING CHIP %f %f", position.x, position.y);
			load(position, { 1.0f, 1.0f }, 0.0f);
		}
	~Chip() = default;

	void load(Vector2 position, Vector2 scale, float rotation);
	void reset(Hex::Point, Vector2 position, Vector2 scale, float rotation, int value);

	ChipState getState() const;
	Vector2 getPosition() const;

	void render() const;

	void update();

	void enable();
	void disable();
	bool active() const;
	bool available() const;

	void unload();
};
