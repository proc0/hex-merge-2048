#pragma once

#include "config.hpp"
#include "type.hpp"
#include "hex.hpp"

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
	int framePropsActive = 0;

	bool enabled;
	bool merged = false;
	bool absorbed = false;

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

	State::Chip state = State::Chip::READY;

public:
	Chip(Hex::Point hex, Vector2 position, int id_, int value, bool active = false): 
		currentHex(hex), 
		targetHex(hex), 
		id(id_),
		value(value),
		enabled(active) {
    		// TraceLog(LOG_INFO, "CREATING CHIP %d: %f %f", value, position.x, position.y);
			load(position);
		}
	~Chip() = default;

	bool operator==(Chip& other) {
		return this->value == other.value;
	}

	void load(Vector2 position);
	void reload(Vector2 position, Vector2 scale, float size, float rotation, Color color);
	// void reset(Hex::Point, Vector2 position, Vector2 scale, float rotation, int value);
	void clear();
	
	int getId() const;
	int getValue() const;
	Hex::Point getCurrentHex() const;
	Vector2 getPosition() const;
	Vector2 getTargetPosition() const;
	bool hasAbsorbed() const;

	void addValue(int);
	void setValue(int);
	void setCurrentHex(Hex::Point);
	void setPosition(Vector2);
	void setScale(Vector2);
	void setSize(float);
	void setRotation(float);
	void setColor(Color);
	
	void render() const;

	State::Chip update();
	int merge(Chip& other);
	void place(Hex::Point, Vector2 position, int value);
	void move(Hex::Point, Vector2 position);

	void enable();
	void disable();
	bool active() const;
	bool available() const;

	void resize();
	void unload();
};
