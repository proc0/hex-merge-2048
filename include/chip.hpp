#pragma once

#include "config.hpp"
#include "type.hpp"
#include "tool.hpp"
#include "hex.hpp"
#include "anime.hpp"

#include "raylib.h"

#include <array>

#define DEFAULT_CHIP_COLOR LIGHTGRAY
#define DEFAULT_CHIP_FONT_COLOR RAYWHITE

// WARNING: must equal the properties enum size
#define PROP_COUNT 10

class Chip {
	enum PROPERTY {
		X,
		Y,
		SCALE,
		FONT_X,
		FONT_Y,
		ROTATION,
		COLOR_R,
		COLOR_G,
		COLOR_B,
		COLOR_A
	};
	static constexpr std::array<Anime::Curve, PROP_COUNT> curveSelect{
		Anime::Curve::EASE_IN_OUT_CUBIC,
		Anime::Curve::EASE_IN_OUT_CUBIC,
		Anime::Curve::EASE_IN_QUAD,
		Anime::Curve::EASE_IN_OUT_BACK,
		Anime::Curve::EASE_IN_QUAD,
		Anime::Curve::EASE_IN_QUAD,
		Anime::Curve::EASE_IN_QUAD,
		Anime::Curve::EASE_IN_QUAD,
		Anime::Curve::EASE_IN_QUAD,
		Anime::Curve::EASE_IN_QUAD,
	};
	static constexpr std::array<float, PROP_COUNT> duration{
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
	};
	// place effects constant values 
	static constexpr StackMap<int, float, 1> configSourcePlaceEffect{{
		{ SCALE, 1.1f },
	}};
	// move effects constant values
	static constexpr StackMap<int, float, 1> configPropsMoveEffect{{
		{ SCALE, 1.08f },
	}};
	// prop config maps that change at runtime
	StackMap<int, float, 2> configMovePropTargets;
	StackMap<int, float, 2> configMergePropDelay;
	
	std::array<float, PROP_COUNT> source{0};
	std::array<float, PROP_COUNT> actual{0};
	std::array<float, PROP_COUNT> target{0};
	std::array<float, PROP_COUNT> elapsed{0};
	std::array<int,   PROP_COUNT> animate{0};

	// StackMap<int, float, 2> moveTargets;
	// StackMap<int, float, 2> delayMoveSources;
	// StackMap<int, float, 2> fontPosTargets;

	Hex::Point hex;

	Vector2 size = { HEX_SIZE, HEX_SIZE };
	int fontSize = CHIP_FONT_SIZE;

	const int id;
	int value = 0;
	int nextValue = 0;
	int animatePropCount = 0;

	State::Chip state = State::Chip::READY;

	bool enabled;
	bool merged = false;
	bool absorbed = false;

public:

	Chip(Hex::Point point, Vector2 position, int chipId, int chipValue, bool active = false): 
		hex(point), 
		id(chipId),
		value(chipValue),
		nextValue(chipValue),
		enabled(active) {
			load(position);
			if (active) place(hex, position, chipValue);
		}
	~Chip() = default;

	bool operator==(Chip& other) {
		return this->value == other.value;
	}

	void load(Vector2 position);
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
	void setProps(FlatMapView<int, float> propMap, bool setSource, bool setActual, bool setTarget);
	void setPosition(Vector2);
	void setFontSize(float fontSize);
	void setSize(Vector2 hexSize);
	void setHex(Hex::Point);
	void setValue(int);
	void addValue(int);
	// getters
	int getId() const;
	int getValue() const;
	Hex::Point getHex() const;
	Vector2 getPosition() const;
	Vector2 getTargetPosition() const;
	float getFontSize() const;
	// query
	bool hasAbsorbed() const;
	void enable();
	void disable();
	bool active() const;
	bool available() const;

	void resize(Vector2 newSize, int newFontSize);
	void unload();
};
