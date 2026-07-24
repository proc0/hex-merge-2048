#include "chip.hpp"
#include "config.hpp"
#include "tool.hpp"

#include "raylib.h"
#include "raymath.h"
#include "type.hpp"

void Chip::load(Vector2 position) {
	float fontWidth = MeasureText(TextFormat("%d", value), CHIP_FONT_SIZE);
	StackMap<int, float, PROP_COUNT> configLoadProps = {
		{ X, position.x },
		{ Y, position.y },
		{ SCALE, 1.0f },
		{ FONT_X, fontWidth*-0.5f },
		{ FONT_Y, CHIP_FONT_SIZE*-0.5f },
		{ ROTATION, 0.0f },
		{ COLOR_R, DEFAULT_CHIP_COLOR.r },
		{ COLOR_G, DEFAULT_CHIP_COLOR.g },
		{ COLOR_B, DEFAULT_CHIP_COLOR.b },
		{ COLOR_A, DEFAULT_CHIP_COLOR.a },
	};

	setProps({{ configLoadProps.data.data(), configLoadProps.size }}, true, true, true);
}

void Chip::reset() {
	enabled = false;
	absorbed = false;
	merged = false;

	for (int prop = 0; prop < PROP_COUNT; ++prop) {
		animate[prop] = 0;
		elapsed[prop] = 0;
	}

	animatePropCount = 0;
	state = State::Chip::READY;
}

void Chip::sync() {
	// delayed value application
	applyPropChanges(syncProps);

	// reset flags
	if (merged) {
		merged = false;
		enabled = false;
	}

	if (absorbed) {
		value = nextValue;
		absorbed = false;
	}
}

void Chip::render() const {
	float radius = size.x*actual[SCALE];
	DrawPoly({ actual[X], actual[Y] }, 6, radius, actual[ROTATION], { static_cast<unsigned char>(actual[COLOR_R]), static_cast<unsigned char>(actual[COLOR_G]), static_cast<unsigned char>(actual[COLOR_B]), static_cast<unsigned char>(actual[COLOR_A]) });
	DrawPolyLinesEx({ actual[X], actual[Y] }, 6, radius, actual[ROTATION], actual[SCALE], BLACK);
	
	DrawText(TextFormat("%d", value), actual[X]+actual[FONT_X], actual[Y]+actual[FONT_Y], fontSize*actual[SCALE], DEFAULT_CHIP_FONT_COLOR);
}

State::Chip Chip::update() {
	// animate props
	float frameTime = GetFrameTime();
	for (int prop = 0; prop < PROP_COUNT; ++prop) {
		int& curveIndex = animate[prop];
		// then it counts the current frame based on 
		// the total values in the animation function array
		if (curveIndex > 0) {
			// update ellapsed animation time
			elapsed[prop] += frameTime;

			// calculate progress based on the animation duration
			float progress = elapsed[prop]/duration[prop];
			// clamp
			bool finished = false;
			if (progress >= 1.0f) {
				elapsed[prop] = 1.0f;
				finished = true;
			}
			
			// calculating resulting animation index from the easing curve
			curveIndex = static_cast<int>(progress * ANIME_CURVE_MAX_INDEX);
			// clamp to 1 because 0 means end of animation
			if (curveIndex < 1) curveIndex = 1;
			if (curveIndex > ANIME_CURVE_MAX_INDEX) curveIndex = ANIME_CURVE_MAX_INDEX;
			
			// update the property value
			auto& curve = ANIME_CURVES[curveSelect[prop]];
			actual[prop] = Lerp(source[prop], target[prop], curve[curveIndex]);

			// on the last frame reset back to 'off'
			if (finished || curveIndex >= ANIME_CURVE_MAX_INDEX) {
				curveIndex = 0;
				elapsed[prop] = 0;
				// global counter of all props
				// currently being animated
				animatePropCount--;
			}
		}
	}

	if (state == State::Chip::MOVING && animatePropCount <= 0) {
		state = State::Chip::READY;
	}

	return state;
}

void Chip::place(Hex::Point point, Vector2 position, int newValue) {
	hex = point;
	value = newValue;
	nextValue = newValue;
	setPosition(position);
	enable();

	// update font of potential new value
	setFontSize(fontSize);
	// animate chip placement, scale and font scale
	animatePropSources({{ configSourcePlaceEffect.data.data(), configSourcePlaceEffect.size }});
}

// move the chip within the hex board
void Chip::translate(Hex::Point point, Vector2 position) {
	hex = point;

	// WARNING: StackMap has hardcoded ITEM CAPACITY: 2
	configMovePropTargets.insert(X, position.x);
	configMovePropTargets.insert(Y, position.y);
	// animate movement by setting the source and target on config props given
	animatePropTargets({{ configMovePropTargets.data.data(), configMovePropTargets.size }});

	// NOTE: these are not animated because they would require
	// prop value changes in the middle of animating (not supported)
	// set this value on the current props, to later be overwritten by target
	setProps({{ configPropsMoveEffect.data.data(), configPropsMoveEffect.size }}, false, true, false);
}

int Chip::merge(Chip& other) {
	nextValue += other.value;
	other.translate(hex, getTargetPosition());
	other.merged = true;
	absorbed = true;

	// update font properties 
	float fontWidth = MeasureText(TextFormat("%d", nextValue), fontSize);
	configMergePropDelay.insert(FONT_X, fontWidth*-0.5f);
	configMergePropDelay.insert(FONT_Y, fontSize*-0.5f);
	// delay prop changes to allow the merging chip to move before changing,
	// similar mechanism as animating, but there are no intermediate values
	delayPropChanges({{ configMergePropDelay.data.data(), configMergePropDelay.size }});

	return nextValue;
}

// ANIMATION METHODS
// ---------------------------------------------
// NOTES: there are two kinds of property changes
// 1. Property changes that animate
// These will set current to source, then set the new value on target
// Then the animation frame flags are set to one in frame array
// Then the global animation flag counter is incremented by how many props were set (animatePropCount)
// Along with setting the state to MOVING, World will also know when Chip is MOVING and load it on its own array to process
// It will keep calling update where the props will be Lerped until animatePropCount is 0, then it chagnes back to READY

// 2. Property changes that do not animate
// These just need to be delayed applied
// First you set current to source, then set the new value on target (technically source is not needed as there is no animation)
// Note that no animation flag is set, or state set to MOVING either, or animatePropCount incremented
// Then in a sync function, the properties update according to the configuration, the target is set to current etc.

// Summary: Two kinds of proprety changes, ones that need animated, ones that just need delayed
// There are two kinds of animations, going from source to target, and from target to source, 
// in the sense that one of them will have the changed property, and either the property will 
// animate into the target, i.e. source (unchanged) -> target (new value), or animate away from source,
// i.e. source (new value) -> target (unchanged)
// current value will store the intermediate Lerp value, or in the case of a delay, do nothing

void Chip::applyPropChanges(const std::span<const int> syncKeys) {

	auto it = syncKeys.begin();
	while(it != syncKeys.end()) {
		int key = *it;

		actual[key] = target[key];

		it++;
	}
}

void Chip::delayPropChanges(FlatMapView<int, float> targetMap) {

	auto it = targetMap.data.begin();
	while(it != targetMap.data.end()) {
		int key = it->first;
		float targetVal = it->second;
		
		source[key] = actual[key];
		target[key] = targetVal;

		it++;
	}
}

void Chip::animatePropSources(FlatMapView<int, float> sourceMap) {

	auto it = sourceMap.data.begin();
	while(it != sourceMap.data.end()) {
		int key = it->first;
		float sourceVal = it->second;
		
		source[key] = sourceVal;
		target[key] = actual[key];
		animate[key] = 1;

		it++;
		animatePropCount++;
	}

	state = State::Chip::MOVING;
}

void Chip::animatePropTargets(FlatMapView<int, float> targetMap) {

	auto it = targetMap.data.begin();
	while(it != targetMap.data.end()) {
		int key = it->first;
		float targetVal = it->second;
		
		source[key] = actual[key];
		target[key] = targetVal;
		animate[key] = 1;

		it++;
		animatePropCount++;
	}

	state = State::Chip::MOVING;
}

// flexible setter that sets all three prop arrays give a StackMap View of variable size
void Chip::setProps(FlatMapView<int, float> propMap, bool setSource, bool setActual, bool setTarget) {

	auto it = propMap.data.begin();
	while(it != propMap.data.end()) {
		int key = it->first;
		float val = it->second;

		if (setSource) {
			source[key] = val;
		}
		
		if (setActual) {
			actual[key] = val;
		}

		if (setTarget) {
			target[key] = val;
		}

		it++;
	}
}

// Setters

void Chip::setPosition(Vector2 position) {
	actual[X] = position.x;	
	source[X] = position.x;	
	target[X] = position.x;

	actual[Y] = position.y;	
	source[Y] = position.y;	
	target[Y] = position.y;
}

void Chip::setFontSize(float newFontSize) {
	fontSize = newFontSize;

	float fontWidth = MeasureText(TextFormat("%d", value), newFontSize);
	float fontX = fontWidth*-0.5f;
	float fontY = newFontSize*-0.5f;

	actual[FONT_X] = fontX;	
	source[FONT_X] = fontX;	
	target[FONT_X] = fontX;

	actual[FONT_Y] = fontY;	
	source[FONT_Y] = fontY;	
	target[FONT_Y] = fontY;
}

void Chip::setSize(Vector2 chipSize) {
	size = chipSize;
}

void Chip::setHex(Hex::Point point) {
	hex = point;
}

void Chip::setValue(int val) {
	value = val;
}

void Chip::addValue(int val) {
	value += val;
}

// Getters

int Chip::getId() const {
	return id;
}

int Chip::getValue() const {
	return value;
}

Hex::Point Chip::getHex() const {
	return hex;
}

Vector2 Chip::getPosition() const {
	return { actual[X], actual[Y] };
}

Vector2 Chip::getTargetPosition() const {
	return { target[X], target[Y] };
}

float Chip::getFontSize() const {
	return fontSize;
}

void Chip::enable() {
	enabled = true;
}

void Chip::disable() {
	enabled = false;
}

bool Chip::hasAbsorbed() const {
	return absorbed;
}

bool Chip::active() const {
	return enabled;
}

bool Chip::available() const {
	return !enabled;
}

void Chip::resize(Vector2 newSize, int newFontSize) {
	size = newSize;
	setFontSize(newFontSize);
}

void Chip::unload() {

}
