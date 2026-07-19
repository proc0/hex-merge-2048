#include "chip.hpp"

#include "raylib.h"

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

void Chip::place(Hex::Point hex, Vector2 position, int val) {
	setPosition(position);
	currentHex = hex;
	value = val;
	enable();
}

Hex::Point Chip::getCurrentHex() const {
	return currentHex;
}

Vector2 Chip::getPosition() const {
	return { current[POSX], current[POSY] };
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