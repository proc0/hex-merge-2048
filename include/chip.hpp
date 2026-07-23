#pragma once

#include "config.hpp"
#include "type.hpp"
#include "hex.hpp"
#include "tool.hpp"

#include "raylib.h"

#include <array>

// WARNING: must equal the properties enum size
#define PROPS_SIZE 14

class Chip {
	// TODO: name the enum and refactor
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

	std::array<float, PROPS_SIZE> source{0};
	std::array<float, PROPS_SIZE> target{0};
	std::array<float, PROPS_SIZE> current{0};
	std::array<int, PROPS_SIZE> frame{0};
	std::array<float, PROPS_SIZE> animEllapsed{0};
	static constexpr std::array<float, PROPS_SIZE> animDuration{
		0.3f,
		0.3f,
		0.2f,
		0.2f,
		0.2f,
		0.2f,
		0.2f,
		0.2f,
		0.2f,
		0.2f,
		0.2f,
		0.2f,
		0.2f,
		0.2f,		
	};
	static constexpr std::array<ANIMATION::FUNC, PROPS_SIZE> animIndex{
		ANIMATION::FUNC::EASE_IN_OUT_CUBIC,
		ANIMATION::FUNC::EASE_IN_OUT_CUBIC,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_OUT_BACK,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_QUAD,
		ANIMATION::FUNC::EASE_IN_QUAD,
	};

	Hex::Point currentHex;

	Color primaryColor = LIGHTGRAY;
	Color secondaryColor = RAYWHITE;

	const int id;
	int value = 0;
	int nextValue = 0;
	int size = HEX_SIZE;
	int framePropsActive = 0;

	// StackMap<int, float, 2> moveTargets;
	// StackMap<int, float, 2> delayMoveSources;
	// StackMap<int, float, 2> fontPosTargets;

	bool enabled;
	bool merged = false;
	bool absorbed = false;

	State::Chip state = State::Chip::READY;

public:
	// TODO: hexSize is passed in due to window resize, refactor constructor params and add a 
	// resize function to call right after it is constructed by World
	Chip(Hex::Point hex, Vector2 hexSize, Vector2 position, int id_, int val, bool active = false): 
		currentHex(hex), 
		id(id_),
		value(val),
		nextValue(val),
		enabled(active) {
			load(hexSize, position);
			if (active) place(hex, position, val);
		}
	~Chip() = default;

	bool operator==(Chip& other) {
		return this->value == other.value;
	}

	void load(Vector2 hexSize, Vector2 position);
	void reset();
	void sync();

	void render() const;
	State::Chip update();

	void place(Hex::Point, Vector2 position, int val);
	void translate(Hex::Point, Vector2 position);
	int merge(Chip& other);
	// animation
	void animatePropTargets(FlatMapView<int, float>);
	void animatePropSources(FlatMapView<int, float>);
	void delayPropChanges(FlatMapView<int, float>);
	void applyPropChanges(const std::span<const int>);
	// setters
	void setProps(FlatMapView<int, float> propMap, bool setSource, bool setCurrent, bool setTarget);
	void setPosition(Vector2);
	void setFontSize(float fontSize);
	void setSize(float);
	void setCurrentHex(Hex::Point);
	void setValue(int);
	void addValue(int);
	// getters
	int getId() const;
	int getValue() const;
	Hex::Point getCurrentHex() const;
	Vector2 getPosition() const;
	Vector2 getTargetPosition() const;
	float getFontSize() const;
	// query
	bool hasAbsorbed() const;
	void enable();
	void disable();
	bool active() const;
	bool available() const;

	void resize();
	void unload();
};
