#pragma once

#include "config.hpp"
#include "type.hpp"
#include "hex.hpp"

#include "raylib.h"

#include <array>

// WARNING: must equal the properties enum size
#define PROPS_SIZE 14

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
	int nextValue = 0;
	int size = HEX_SIZE;
	int framePropsActive = 0;

	float fontXOne = 0;
	float fontXTwo = 0;
	float fontXThree = 0;
	float fontXFour = 0;

	bool enabled;
	bool merged = false;
	bool absorbed = false;

	enum {
		POSX,
		POSY,
		SIZE,
		SCALE,
		FONTX,
		FONTY,
		FONTSIZE,
		FONTSCALE,
		BORDERSIZE,
		ROT,
		COLR,
		COLG,
		COLB,
		COLA
	};

	State::Chip state = State::Chip::READY;

public:
	Chip(Hex::Point hex, Vector2 position, int id_, int val, bool active = false): 
		currentHex(hex), 
		targetHex(hex), 
		id(id_),
		value(val),
		nextValue(val),
		enabled(active) {
    		// TraceLog(LOG_INFO, "CREATING CHIP %d: %f %f", value, position.x, position.y);
			load(position);
			if (active) place(hex, position, val);
		}
	~Chip() = default;

	bool operator==(Chip& other) {
		return this->value == other.value;
	}

	void load(Vector2 position);
	// void reload(Vector2 position, float scale, float size, float fontSize, float rotation, Color color);
	void reset();
	
	int getId() const;
	int getValue() const;
	Hex::Point getCurrentHex() const;
	Vector2 getPosition() const;
	Vector2 getTargetPosition() const;
	float getFontSize() const;
	bool hasAbsorbed() const;

	void addValue(int);
	void setValue(int);
	void setCurrentHex(Hex::Point);
	void setPosition(Vector2);
	void setScale(float);
	void setFontProps(Vector2 position, float fontSize, float fontScale);
	void setFontTargetProps(Vector2 position, float fontSize, float fontScale);
	void setFontSyncProps();
	void setSize(float);
	void setFontSize(float);
	void setRotation(float);
	void setColor(Color);
	
	void render() const;

	State::Chip update();
	void sync();
	int merge(Chip& other);
	void place(Hex::Point, Vector2 position, int val);
	void move(Hex::Point, Vector2 position);
	void updateFont(float fontSize);

	void enable();
	void disable();
	bool active() const;
	bool available() const;

	void resize();
	void unload();
};
