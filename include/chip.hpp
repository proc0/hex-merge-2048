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

	const int id;
	int value = 0;
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
	Chip(Hex::Point hex, Vector2 position, int id_, int value, bool active = false): 
		currentHex(hex), 
		targetHex(hex), 
		id(id_),
		value(value),
		enabled(active) {
    		TraceLog(LOG_INFO, "CREATING CHIP %d: %f %f", value, position.x, position.y);
			load(position);
		}
	~Chip() = default;

	void load(Vector2 position);
	void reload(Vector2 position, Vector2 scale, float size, float rotation, Color color);
	// void reset(Hex::Point, Vector2 position, Vector2 scale, float rotation, int value);
	void place(Hex::Point, Vector2 position, int value);

	Hex::Point getCurrentHex() const;
	void setCurrentHex(Hex::Point);
	Vector2 getPosition() const;
	int getId() const;
	int getValue() const;
	void setPosition(Vector2);
	void setScale(Vector2);
	void setSize(float);
	void setRotation(float);
	void setColor(Color);
	
	void render() const;

	void update();

	void enable();
	void disable();
	bool active() const;
	bool available() const;

	void resize();
	void unload();
};
