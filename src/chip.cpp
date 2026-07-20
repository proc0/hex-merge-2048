#include "chip.hpp"
#include "tool.hpp"

#include "raylib.h"
#include "raymath.h"
#include "type.hpp"

void Chip::load(Vector2 position) {
	setPosition(position);
	setScale({ 1.0f, 1.0f });
	setSize(HEX_SIZE);
	setRotation(0.0f);
	setColor(primaryColor);
}

void Chip::reload(Vector2 position, Vector2 scale, float size, float rotation, Color color) {
	setPosition(position);
	setScale(scale);
	setSize(size);
	setRotation(rotation);
	setColor(color);
}

// void Chip::reset(Hex::Point hex, Vector2 position, Vector2 scale, float rotation, int val) {
// 	reload(position, scale, size, rotation, primaryColor);
// 	currentHex = hex;
// 	value = val;
// 	enable();
// }

void Chip::clear() {
	if (absorbed) {
		value = nextValue;
		absorbed = false;
	}
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
		state = State::Chip::READY;
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
}

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

	framePropsActive = 2;
}

int Chip::merge(Chip& other) {
	nextValue += other.value;
	other.move(currentHex, getTargetPosition());
	other.merged = true;
	absorbed = true;

	return nextValue;
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

void Chip::setScale(Vector2 scale) {
	current[SCALEX] = scale.x;	
	source[SCALEX] = scale.x;	
	target[SCALEX] = scale.x;

	current[SCALEY] = scale.y;	
	source[SCALEY] = scale.y;	
	target[SCALEY] = scale.y;
}

void Chip::setSize(float size) {
	current[SIZE] = size;	
	source[SIZE] = size;	
	target[SIZE] = size;
}

void Chip::setRotation(float rotation) {
	current[ROT] = rotation;	
	source[ROT] = rotation;	
	target[ROT] = rotation;
}

void Chip::setColor(Color color) {
	current[COL1] = color.r;	
	source[COL1] = color.r;	
	target[COL1] = color.r;

	current[COL2] = color.g;	
	source[COL2] = color.g;	
	target[COL2] = color.g;

	current[COL3] = color.b;	
	source[COL3] = color.b;	
	target[COL3] = color.b;

	current[COL4] = color.a;	
	source[COL4] = color.a;	
	target[COL4] = color.a;
}

void Chip::render() const {
	DrawPoly({ current[POSX], current[POSY] }, 6, current[SIZE], current[ROT], { static_cast<unsigned char>(current[COL1]), static_cast<unsigned char>(current[COL2]), static_cast<unsigned char>(current[COL3]), static_cast<unsigned char>(current[COL4]) });
	DrawPolyLines({ current[POSX], current[POSY] }, 6, current[SIZE], current[ROT], BLACK);
	
	const char* sigilValue = TextFormat("%d", value);
	float fontWidth = MeasureText(sigilValue, 42);
	DrawText(sigilValue, current[POSX]-fontWidth*0.5f, current[POSY]-21.0f, 42, secondaryColor);
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