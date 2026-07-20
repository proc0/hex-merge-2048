#include "grid.hpp"

#include "type.hpp"

#include "raylib.h"

#include <cmath>

void Grid::load() {
	create(extent);

	loadTarget();
}

void Grid::loadTarget() {
    // Render texture to draw, enables window scaling
    // NOTE: If window is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(window.width, window.height);
    targetSource = { 0, 0, static_cast<float>(target.texture.width), -static_cast<float>(target.texture.height) };
    targetDestination = { 0, 0, static_cast<float>(target.texture.width), static_cast<float>(target.texture.height) };
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    BeginTextureMode(target);
        renderGrid();
    EndTextureMode();
}

// k = extent = number of layers
void Grid::create(int k) {
	for (int q = -k; q <= k; q++) {
		int r1 = fmax(-k, -q - k);
		int r2 = fmin(k, -q + k);
		// bool isRowEdge = q == -k || q == k;
		for (int r = r1; r <= r2; r++) {
			Hex::Point hex = {q, r};
			// bool isColEdge = r == r1 || r == r2;
			map.insert({ hex, { project(hex, origin, unit), 0 } });
		}
	}
}

void Grid::reset() {
	for (auto &[hex, state] : map) {
		state.key = 0;
	}
}

void Grid::render() const {
    DrawTexturePro(target.texture, targetSource, targetDestination, Vector2({}), 0.0f, WHITE);
    // render hex effects
	for (auto &[hex, state] : map) {
		if (state.key == 0) continue;
		DrawPoly(state.position, 6, unit.x, 0.0f, { 253, 249, 0, 150 });
	}
}

void Grid::renderGrid() const {
	for (auto &[hex, state] : map) {
		renderHex(hex, state);
	}
}

void Grid::renderHex(const Hex::Point& point, const HexState& state) const {
	DrawPoly(state.position, 6, unit.x, 0.0f, colorHex);
	DrawPolyLinesEx(state.position, 6, unit.x, 0.0f, 2.0f, colorLine);

	// const char *pointLabel = TextFormat("(%d, %d, %d)", point.q, point.r, point.s);
	// DrawText(pointLabel, state.position.x - 30.0f, state.position.y - 9.0f, 18, BLACK);
}

HexState Grid::getState(Hex::Point hex) const {
	// TODO: change to map[hex] and/or add DEBUG guard
	return map.at(hex);
}

Vector2 Grid::getPosition(Hex::Point hex) const {
	// TODO: change to map[hex] and/or add DEBUG guard
	return map.at(hex).position;
}

Vector2 Grid::getUnit() const {
	return unit;
}

void Grid::place(Hex::Point point, int key) {
	// TODO: change to map[hex] and/or add DEBUG guard
	HexState& state = map.at(point);
	state.key = key;
}

void Grid::clear(Hex::Point point) {
	// TODO: change to map[hex] and/or add DEBUG guard
	HexState& state = map.at(point);
	state.key = 0;
}

Hex::Point Grid::walk(Hex::Basis direction, Hex::Point departure) const {
	Hex::Point destination = departure + direction;

	return inside(destination) ? destination : departure;
}

// get a corner based on unit direction hex
Hex::Point Grid::corner(Hex::Point source) const {
	Hex::Point target = Hex::multiply(source, extent);

	return inside(target) ? target : source;
}

bool Grid::inside(Hex::Point point) const {
	return abs(point.q) <= extent && abs(point.r) <= extent && abs(point.s) <= extent;
}

// obtain the edge of the hex grid in a given direction
// necessary because chips can move along edges if the direction is 
// not towards that edge (i.e. chip is on left edge moving up)
// WARNING: the direction should be a unit hex!
bool Grid::walkEdge(Hex::Basis dir, Hex::Point hex) const {
	// NOTE: this detects the edge specifically, no whether it's inside the grid 
	if (dir.q == 0) {
		return hex.r == dir.r * extent || hex.s == dir.s * extent;
	} else if (dir.r == 0) {
		return hex.q == dir.q * extent || hex.s == dir.s * extent;
	} else if (dir.s == 0) {
		return hex.q == dir.q * extent || hex.r == dir.r * extent;
	}

	return false;
}

Hex::Point Grid::findAny() const {
	Hex::Point result = Hex::Absurd;

	for (auto &[hex, state] : map) {
		if (state.key == 0) {
			result = hex;
			break;
		}
	}

	return result;
}

Hex::Point Grid::findCenter() const {
	Hex::Point result = Hex::Absurd;

	for (auto &[hex, state] : map) {
		if (abs(hex.q) + abs(hex.r) + abs(hex.s) <= extent && state.key == 0) {
			result = hex;
			break;
		}
	}

	if (result == Hex::Absurd) {
		result = findAny();
	}

	return result;
}

Hex::Point Grid::findRandom() const {
	Hex::Point candidate = Hex::Absurd;

	bool found = false;
	int maxRetry = 15;
	while (maxRetry > 0) {
		int hq = GetRandomValue(-extent, extent);
		int hr = GetRandomValue(-extent, extent);
		int hs = -hq - hr;
		// satisfy hex grid constraint
        if (hq + hr + hs != 0) continue;

		candidate = Hex::Point(hq, hr, hs);
		// found random hex if it is inside and not occupied
		if (inside(candidate) && map.at(candidate).key == 0) {
			found = true;
			break;
		}

		maxRetry--;
	}

	if (!found) {
		candidate = findCenter();
	}

	return candidate;
}


bool Grid::vacant(Hex::Point hex) const {
	return map.at(hex).key == 0;
}

bool Grid::occupied(Hex::Point hex) const {
	return map.at(hex).key > 0;
}

bool Grid::filled() const {
	bool isFull = true;
	for (auto& [hex, state] : map) {
		if (state.key == 0) {
			isFull = false;
			break;
		}
	}

	return isFull;
}

int Grid::size() const {
	return 1 + 6 * summation(extent);
}

void Grid::resize(int width, int height) {
	unit.x = unit.y = window.scale(HEX_SIZE);
	origin.x = window.halfWidthf;
	origin.y = window.halfHeightf;

	for (auto &[point, state] : map) {
		state.position = project(point, origin, unit);
	}

	loadTarget();
}

void Grid::unload() {
    UnloadRenderTexture(target);
}
