#include "chip.hpp"
#include "tool.hpp"

#include "raylib.h"
#include "raymath.h"
#include "type.hpp"

void Chip::load(Vector2 position) {
	setPosition(position);
	setScale(1.0f);
	setSize(HEX_SIZE);

	float fontXOne = MeasureText("2", CHIP_FONT_SIZE);
	setFontProps({ fontXOne*-0.5f, CHIP_FONT_SIZE*-0.5f }, CHIP_FONT_SIZE, 1.0f);

	source[BORDERSIZE] = 1.0f;
	current[BORDERSIZE] = 1.0f;
	target[BORDERSIZE] = 1.0f;

	setRotation(0.0f);
	setColor(primaryColor);
}

// void Chip::reload(Vector2 position, float scale, float size, float fontSize, float rotation, Color color) {
// 	setPosition(position);
// 	setScale(scale);
// 	setSize(size);
// 	setFontSize(size);
// 	setRotation(rotation);
// 	setColor(color);
// }

void Chip::reset() {
	state = State::Chip::READY;
	enabled = false;
	absorbed = false;
	merged = false;
}

void Chip::sync() {
	if (absorbed) {
		value = nextValue;
		absorbed = false;
		setFontSyncProps();
	}
}

void Chip::render() const {
	float radius = current[SIZE]*current[SCALE];
	DrawPoly({ current[POSX], current[POSY] }, 6, radius, current[ROT], { static_cast<unsigned char>(current[COLR]), static_cast<unsigned char>(current[COLG]), static_cast<unsigned char>(current[COLB]), static_cast<unsigned char>(current[COLA]) });
	DrawPolyLinesEx({ current[POSX], current[POSY] }, 6, radius, current[ROT], current[BORDERSIZE], BLACK);
	
	const char* textValue = TextFormat("%d", value);
	// float fontWidth = MeasureText(textValue, current[FONTSIZE]);
	DrawText(textValue, current[POSX]+current[FONTX], current[POSY]+current[FONTY], current[FONTSIZE]*current[FONTSCALE], secondaryColor);
}

State::Chip Chip::update() {
	// bool stillMoving = state == State::Chip::MOVING;
	for (int i = 0; i < PROPS_SIZE; ++i) {
		int& currentFrame = frame[i];
		if (currentFrame > 0) {
			// float progress = ANIM_EASE_IN_QUAD[i];
			current[i] = Lerp(source[i], target[i], ANIM_EASE_IN_QUAD[currentFrame]);
			currentFrame++;
			if (currentFrame >= ANIM_EASE_IN_QUAD_MAX_IDX) {
				currentFrame = 0;
				framePropsActive--;
				// current[i] = target[i];
			}
		}
	}

	if (state == State::Chip::MOVING && framePropsActive <= 0) {
		// NOTE: this only gets called for MOVING chips
		// if non-moving chips need prop sync, use sync()
		// TODO: refactor this pattern into more consistent/obvious
		state = State::Chip::READY;
		// sync prop states
		// TODO: abstract this pattern
		// 1. set target props in merge or move
		// 2. current props get copied to source props
		// 3. (optional) source and target are lerped into current
		// 4. at some endpoint (i.e. right here) current is set to target
		// TODO: is this block for syncing even needed if it can move to sync()?
		current[SCALE] = target[SCALE];
		setFontSyncProps();
		current[BORDERSIZE] = target[BORDERSIZE];

		if (merged) {
			merged = false;
			enabled = false;
		}
	}

	return state;
}

void Chip::place(Hex::Point hex, Vector2 position, int val) {
	setPosition(position);
	currentHex = hex;
	value = val;
	nextValue = val;
	enable();

	// update font properties 
	// float fontWidth = MeasureText(TextFormat("%d", value), current[FONTSIZE]);
	// setFontProps({ fontWidth*-0.5f, current[FONTSIZE]*-0.5f }, current[FONTSIZE], current[FONTSCALE]);
	updateFont(current[FONTSIZE]);

	state = State::Chip::MOVING;

	source[SCALE] = 1.12f;
	current[SCALE] = 1.12f;
	target[SCALE] = 1.0f;
	frame[SCALE] = 1;

	source[FONTSCALE] = 1.12f;
	current[FONTSCALE] = 1.12f;
	target[FONTSCALE] = 1.0f;
	frame[FONTSCALE] = 1;

	framePropsActive += 2;
}

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

void Chip::move(Hex::Point hex, Vector2 position) {
	currentHex = hex;
	// current[POSX] = position.x;	
	source[POSX] = current[POSX];	
	target[POSX] = position.x;

	// current[POSY] = position.y;	
	source[POSY] = current[POSY];	
	target[POSY] = position.y;

	state = State::Chip::MOVING;

	frame[POSX] = 1;
	frame[POSY] = 1;

	framePropsActive += 2;

	// none-animated props
	// TODO: abstract this into a 
	// generic setTargetProps of some kind
	current[SCALE] = 1.08f;
	target[SCALE] = 1.0f;

	current[FONTSCALE] = 1.08f;
	target[FONTSCALE] = 1.0f;
	
	current[BORDERSIZE] = 2.5f;
	target[BORDERSIZE] = 1.0f;
}

int Chip::merge(Chip& other) {
	nextValue += other.value;
	other.move(currentHex, getTargetPosition());
	other.merged = true;
	absorbed = true;

	// update font properties 
	float fontWidth = MeasureText(TextFormat("%d", nextValue), current[FONTSIZE]);
	setFontTargetProps({ fontWidth*-0.5f, current[FONTSIZE]*-0.5f }, current[FONTSIZE], current[FONTSCALE]);

	return nextValue;
}

void Chip::updateFont(float fontSize) {

	// update font properties 
	float fontWidth = MeasureText(TextFormat("%d", value), fontSize);
	setFontProps({ fontWidth*-0.5f, fontSize*-0.5f }, fontSize, current[FONTSCALE]);
}

bool Chip::hasAbsorbed() const {
	return absorbed;
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

int Chip::getId() const {
	return id;
}

int Chip::getValue() const {
	return value;
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

void Chip::setPosition(Vector2 position) {
	current[POSX] = position.x;	
	source[POSX] = position.x;	
	target[POSX] = position.x;

	current[POSY] = position.y;	
	source[POSY] = position.y;	
	target[POSY] = position.y;
}

void Chip::setScale(float scale) {
	current[SCALE] = scale;	
	source[SCALE] = scale;	
	target[SCALE] = scale;
}

void Chip::setSize(float size) {
	current[SIZE] = size;	
	source[SIZE] = size;	
	target[SIZE] = size;
}

void Chip::setFontSize(float fontSize) {
	current[FONTSIZE] = fontSize;	
	source[FONTSIZE] = fontSize;	
	target[FONTSIZE] = fontSize;
}

void Chip::setFontProps(Vector2 position, float fontSize, float fontScale) {
	current[FONTX] = position.x;	
	source[FONTX] = position.x;	
	target[FONTX] = position.x;

	current[FONTY] = position.y;	
	source[FONTY] = position.y;	
	target[FONTY] = position.y;

	current[FONTSIZE] = fontSize;	
	source[FONTSIZE] = fontSize;	
	target[FONTSIZE] = fontSize;

	current[FONTSCALE] = fontScale;	
	source[FONTSCALE] = fontScale;	
	target[FONTSCALE] = fontScale;
}

void Chip::setFontSyncProps() {
	current[FONTX] = target[FONTX];	
	current[FONTY] = target[FONTY];	
	// current[FONTSIZE] = target[FONTSIZE];	
	current[FONTSCALE] = target[FONTSCALE];	
}

void Chip::setFontTargetProps(Vector2 position, float fontSize, float fontScale) {
	source[FONTX] = current[FONTX];	
	target[FONTX] = position.x;

	source[FONTY] = current[FONTY];	
	target[FONTY] = position.y;

	source[FONTSIZE] = current[FONTSIZE];	
	target[FONTSIZE] = fontSize;

	source[FONTSCALE] = current[FONTSCALE];	
	target[FONTSCALE] = fontScale;
}

void Chip::setRotation(float rotation) {
	current[ROT] = rotation;	
	source[ROT] = rotation;	
	target[ROT] = rotation;
}

void Chip::setColor(Color color) {
	current[COLR] = color.r;	
	source[COLR] = color.r;	
	target[COLR] = color.r;

	current[COLG] = color.g;	
	source[COLG] = color.g;	
	target[COLG] = color.g;

	current[COLB] = color.b;	
	source[COLB] = color.b;	
	target[COLB] = color.b;

	current[COLA] = color.a;	
	source[COLA] = color.a;	
	target[COLA] = color.a;
}

void Chip::enable() {
	enabled = true;
}

void Chip::disable() {
	enabled = false;
}

bool Chip::active() const {
	return enabled;
}

bool Chip::available() const {
	return !enabled;
}

void Chip::unload() {

}