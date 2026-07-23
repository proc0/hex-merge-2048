#include "chip.hpp"
#include "config.hpp"
#include "tool.hpp"

#include "raylib.h"
#include "raymath.h"
#include "type.hpp"

void Chip::load(Vector2 hexSize, Vector2 position) {
	// initial font width measurement
	float fontXOne = MeasureText("2", CHIP_FONT_SIZE);
	StackMap<int, float, PROPS_SIZE> initProps = {
		{ POSX, position.x },
		{ POSY, position.y },
		{ SCALE, 1.0f },
		// TODO: make size always a vector
		{ SIZE, hexSize.x },
		{ FONTX, fontXOne*-0.5f },
		{ FONTY, CHIP_FONT_SIZE*-0.5f },
		{ FONTSIZE, CHIP_FONT_SIZE },
		{ FONTSCALE, 1.0f },
		{ BORDERSIZE, 1.0f },
		{ ROT, 0.0f },
		{ COLR, primaryColor.r },
		{ COLG, primaryColor.g },
		{ COLB, primaryColor.b },
		{ COLA, primaryColor.a },
	};
	setProps({{ initProps.data.data(), initProps.size }}, true, true, true);
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
	constexpr std::array<int, 6> syncMap = {
		SCALE,
		BORDERSIZE,
		SCALE,
		FONTX,
		FONTY,
		FONTSCALE,
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
	float radius = current[SIZE]*current[SCALE];
	DrawPoly({ current[POSX], current[POSY] }, 6, radius, current[ROT], { static_cast<unsigned char>(current[COLR]), static_cast<unsigned char>(current[COLG]), static_cast<unsigned char>(current[COLB]), static_cast<unsigned char>(current[COLA]) });
	DrawPolyLinesEx({ current[POSX], current[POSY] }, 6, radius, current[ROT], current[BORDERSIZE], BLACK);
	
	DrawText(TextFormat("%d", value), current[POSX]+current[FONTX], current[POSY]+current[FONTY], current[FONTSIZE]*current[FONTSCALE], secondaryColor);
}

State::Chip Chip::update() {
	// animate props
	for (int i = 0; i < PROPS_SIZE; ++i) {
		// NOTE: the first value is a flag
		int& currentFrame = frame[i];
		// then it counts the current frame based on 
		// the total values in the animation function array
		if (currentFrame > 0) {
			auto& animFunction = Animations[animIndex[i]];
			current[i] = Lerp(source[i], target[i], animFunction[currentFrame]);
			// increment frame count
			currentFrame++;
			// on the last frame reset back to 'off'
			if (currentFrame > ANIMATION_MAX_INDEX) {
				currentFrame = 0;
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

void Chip::place(Hex::Point hex, Vector2 position, int val) {
	setPosition(position);
	currentHex = hex;
	value = val;
	nextValue = val;
	enable();

	// update font of potential new value
	setFontSize(current[FONTSIZE]);

	// animate chip placement, scale and font scale
	// TODO: memoize StackMap in member field
	StackMap<int, float, 2> scaledPropSources;
	scaledPropSources.insert(SCALE, 1.1f);
	scaledPropSources.insert(FONTSCALE, 1.1f);
	animatePropSources({{ scaledPropSources.data.data(), scaledPropSources.size }});
}

// move the chip within the hex board
void Chip::translate(Hex::Point hex, Vector2 position) {
	currentHex = hex;

	// animate movement
	// TODO: memoize this in a member field
	StackMap<int, float, 2> moveAnimation;
	moveAnimation.insert(POSX, position.x);
	moveAnimation.insert(POSY, position.y);
	animatePropTargets({{ moveAnimation.data.data(), moveAnimation.size }});

	// set the these props to a different value
	// which then get sync'd after the movement
	// to create a lifting effect
	// NOTE: these are not animated because they would require
	// prop value changes in the middle of animating, not supported
	// TODO: memoize this in a member field
	StackMap<int, float, 3> liftEffect;
	liftEffect.insert(SCALE, 1.08f);
	liftEffect.insert(FONTSCALE, 1.08f);
	liftEffect.insert(BORDERSIZE, 1.08f);
	// set this value on the current props, to later be overwritten by target
	setProps({{ liftEffect.data.data(), liftEffect.size }}, false, true, false);
}

int Chip::merge(Chip& other) {
	nextValue += other.value;
	other.translate(currentHex, getTargetPosition());
	other.merged = true;
	absorbed = true;

	// update font properties 
	float fontWidth = MeasureText(TextFormat("%d", nextValue), current[FONTSIZE]);
	// TODO: memoize this in a member field
	StackMap<int, float, 2> fontPropChanges;
	fontPropChanges.insert(FONTX, fontWidth*-0.5f);
	fontPropChanges.insert(FONTY, current[FONTSIZE]*-0.5f);
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

		current[key] = target[key];

		it++;
	}
}

void Chip::delayPropChanges(FlatMapView<int, float> targetMap) {

	auto it = targetMap.data.begin();
	while(it != targetMap.data.end()) {
		int key = it->first;
		float targetVal = it->second;
		
		source[key] = current[key];
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
		target[key] = current[key];
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
		
		source[key] = current[key];
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
			current[key] = val;
		}

		if (setTarget) {
			target[key] = val;
		}

		it++;
	}
}

// Setters

void Chip::setPosition(Vector2 position) {
	current[POSX] = position.x;	
	source[POSX] = position.x;	
	target[POSX] = position.x;

	current[POSY] = position.y;	
	source[POSY] = position.y;	
	target[POSY] = position.y;
}

void Chip::setFontSize(float fontSize) {
	float fontWidth = MeasureText(TextFormat("%d", value), fontSize);
	float fontX = fontWidth*-0.5f;
	float fontY = fontSize*-0.5f;

	current[FONTX] = fontX;	
	source[FONTX] = fontX;	
	target[FONTX] = fontX;

	current[FONTY] = fontY;	
	source[FONTY] = fontY;	
	target[FONTY] = fontY;

	current[FONTSIZE] = fontSize;	
	source[FONTSIZE] = fontSize;	
	target[FONTSIZE] = fontSize;
}

void Chip::setSize(float size) {
	current[SIZE] = size;	
	source[SIZE] = size;	
	target[SIZE] = size;
}

void Chip::setCurrentHex(Hex::Point hex) {
	currentHex = hex;
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
	return currentHex;
}

Vector2 Chip::getPosition() const {
	return { current[POSX], current[POSY] };
}

Vector2 Chip::getTargetPosition() const {
	return { target[POSX], target[POSY] };
}

float Chip::getFontSize() const {
	return current[FONTSIZE];
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

void Chip::unload() {

}