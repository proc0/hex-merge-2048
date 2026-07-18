#include "chip.hpp"

#include "raylib.h"

void Chip::load(Vector2 position, Vector2 scale, float rotation) {
	current[POSX] = position.x;	
	source[POSX] = position.x;	
	target[POSX] = position.x;

	current[POSY] = position.y;	
	source[POSY] = position.y;	
	target[POSY] = position.y;

	current[SCALEX] = scale.x;	
	source[SCALEX] = scale.x;	
	target[SCALEX] = scale.x;

	current[SCALEY] = scale.y;	
	source[SCALEY] = scale.y;	
	target[SCALEY] = scale.y;

	current[SIZE] = size;	
	source[SIZE] = size;	
	target[SIZE] = size;

	current[ROT] = rotation;	
	source[ROT] = rotation;	
	target[ROT] = rotation;

	current[COL1] = primaryColor.r;	
	source[COL1] = primaryColor.r;	
	target[COL1] = primaryColor.r;

	current[COL2] = primaryColor.g;	
	source[COL2] = primaryColor.g;	
	target[COL2] = primaryColor.g;

	current[COL3] = primaryColor.b;	
	source[COL3] = primaryColor.b;	
	target[COL3] = primaryColor.b;

	current[COL4] = primaryColor.a;	
	source[COL4] = primaryColor.a;	
	target[COL4] = primaryColor.a;
}

void Chip::reset(Hex::Point hex, Vector2 position, Vector2 scale, float rotation, int val) {
	load(position, scale, rotation);
	currentHex = hex;
	state.value = val;
	enable();
}

void Chip::render() const {
	DrawPoly({ current[POSX], current[POSY] }, 6, current[SIZE], 0.0f, { static_cast<unsigned char>(current[COL1]), static_cast<unsigned char>(current[COL2]), static_cast<unsigned char>(current[COL3]), static_cast<unsigned char>(current[COL4]) });
	DrawPolyLines({ current[POSX], current[POSY] }, 6, current[SIZE], 0.0f, BLACK);
	
	const char* sigilValue = TextFormat("%d", state.value);
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