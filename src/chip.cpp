#include "chip.hpp"
#include "config.hpp"
#include "tool.hpp"

#include "raylib.h"
#include "raymath.h"
#include "type.hpp"

void Chip::load(Vector2 position) {
	float fontWidth = MeasureText("2", CHIP_FONT_SIZE);
	StackMap<int, float, PROP_COUNT> loadProps = {
		{ X, position.x },
		{ Y, position.y },
		{ SCALE, 1.0f },
		{ FONT_X, fontWidth*-0.5f },
		{ FONT_Y, CHIP_FONT_SIZE*-0.5f },
		{ ROTATION, 0.0f },
		{ COLOR_R, primaryColor.r },
		{ COLOR_G, primaryColor.g },
		{ COLOR_B, primaryColor.b },
		{ COLOR_A, primaryColor.a },
	};
	setProps({{ loadProps.data.data(), loadProps.size }}, true, true, true);
}

void Chip::reset() {
	enabled = false;
	absorbed = false;
	merged = false;

	for (auto& f : frame) {
		f = 0;
	}
	framePropsActive = 0;
	state = State::Chip::READY;
}

void Chip::sync() {
	// sync all the props that changed
	constexpr std::array<int, 3> syncMap = {
		SCALE,
		FONT_X,
		FONT_Y,
	};

	applyPropChanges(syncMap);

	// reset flags
	if (merged) {
		merged = false;
		enabled = false;
	}
	// delayed value application
	if (absorbed) {
		value = nextValue;
		absorbed = false;
	}
}

void Chip::render() const {
	float radius = size.x*actual[SCALE];
	DrawPoly({ actual[X], actual[Y] }, 6, radius, actual[ROTATION], { static_cast<unsigned char>(actual[COLOR_R]), static_cast<unsigned char>(actual[COLOR_G]), static_cast<unsigned char>(actual[COLOR_B]), static_cast<unsigned char>(actual[COLOR_A]) });
	DrawPolyLinesEx({ actual[X], actual[Y] }, 6, radius, actual[ROTATION], actual[SCALE], BLACK);
	
	DrawText(TextFormat("%d", value), actual[X]+actual[FONT_X], actual[Y]+actual[FONT_Y], fontSize*actual[SCALE], secondaryColor);
}

State::Chip Chip::update() {
	// animate props
	float frameTime = GetFrameTime();
	for (int i = 0; i < PROP_COUNT; ++i) {
		// NOTE: the first value is a flag
		int& currentFrame = frame[i];
		// then it counts the current frame based on 
		// the total values in the animation function array
		if (currentFrame > 0) {
			// update ellapsed animation time
			animEllapsed[i] += frameTime;
			// calculate progress based on the animation duration
			float progress = animEllapsed[i]/animDuration[i];
			// clamp
			bool done = false;
			if (progress >= 1.0f) {
				animEllapsed[i] = 1.0f;
				done = true;
			}
			// calculating resulting animation index from the easing curve
			currentFrame = static_cast<int>(progress*ANIMATION_MAX_INDEX);
			// clamp to 1 because 0 means end of animation
			if (currentFrame < 1) currentFrame = 1;
			if (currentFrame > ANIMATION_MAX_INDEX) currentFrame = ANIMATION_MAX_INDEX;
			// update the property value
			auto& animFunction = Animations[animIndex[i]];
			actual[i] = Lerp(source[i], target[i], animFunction[currentFrame]);
			// on the last frame reset back to 'off'
			if (done || currentFrame >= ANIMATION_MAX_INDEX) {
				currentFrame = 0;
				animEllapsed[i] = 0;
				// global counter of all props
				// currently being animated
				framePropsActive--;
			}
		}
	}

	if (state == State::Chip::MOVING && framePropsActive <= 0) {
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
	// TODO: memoize StackMap in member field
	StackMap<int, float, 1> scaledPropSources;
	scaledPropSources.insert(SCALE, 1.1f);
	// scaledPropSources.insert(FONTSCALE, 1.1f);
	animatePropSources({{ scaledPropSources.data.data(), scaledPropSources.size }});
}

// move the chip within the hex board
void Chip::translate(Hex::Point point, Vector2 position) {
	hex = point;

	// animate movement
	// TODO: memoize this in a member field
	StackMap<int, float, 2> moveAnimation;
	moveAnimation.insert(X, position.x);
	moveAnimation.insert(Y, position.y);
	animatePropTargets({{ moveAnimation.data.data(), moveAnimation.size }});

	// set the these props to a different value
	// which then get sync'd after the movement
	// to create a lifting effect
	// NOTE: these are not animated because they would require
	// prop value changes in the middle of animating, not supported
	// TODO: memoize this in a member field
	StackMap<int, float, 1> liftEffect;
	liftEffect.insert(SCALE, 1.08f);
	// liftEffect.insert(FONTSCALE, 1.08f);
	// liftEffect.insert(BORDERSIZE, 1.08f);
	// set this value on the current props, to later be overwritten by target
	setProps({{ liftEffect.data.data(), liftEffect.size }}, false, true, false);
}

int Chip::merge(Chip& other) {
	nextValue += other.value;
	other.translate(hex, getTargetPosition());
	other.merged = true;
	absorbed = true;

	// update font properties 
	float fontWidth = MeasureText(TextFormat("%d", nextValue), fontSize);
	// TODO: memoize this in a member field
	StackMap<int, float, 2> fontPropChanges;
	fontPropChanges.insert(FONT_X, fontWidth*-0.5f);
	fontPropChanges.insert(FONT_Y, fontSize*-0.5f);
	// delay prop changes to allow the merging chip to move before changing,
	// similar mechanism as animating, but there is not intermediate values
	delayPropChanges({{ fontPropChanges.data.data(), fontPropChanges.size }});

	return nextValue;
}

// ANIMATION METHODS
// ---------------------------------------------
// NOTES: there are two kinds of property changes
// 1. Property changes that animate
// These will set current to source, then set the new value on target
// Then the animation frame flags are set to one in frame array
// Then the global animation flag counter is incremented by how many props were set (framePropsActive)
// Along with setting the state to MOVING, World will also know when Chip is MOVING and load it on its own array to process
// It will keep calling update where the props will be Lerped until framePropsActive is 0, then it chagnes back to READY

// 2. Property changes that do not animate
// These just need to be delayed applied
// First you set current to source, then set the new value on target (technically source is not needed as there is no animation)
// Note that no animation flag is set, or state set to MOVING either, or framePropsActive incremented
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
		frame[key] = 1;

		it++;
		framePropsActive++;
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
		frame[key] = 1;

		it++;
		framePropsActive++;
	}

	state = State::Chip::MOVING;
}

// flexible setter that sets all three prop arrays give a StackMap View of variable size
void Chip::setProps(FlatMapView<int, float> propMap, bool setSource, bool setCurrent, bool setTarget) {

	auto it = propMap.data.begin();
	while(it != propMap.data.end()) {
		int key = it->first;
		float val = it->second;

		if (setSource) {
			source[key] = val;
		}
		
		if (setCurrent) {
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

Hex::Point Chip::getCurrentHex() const {
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
