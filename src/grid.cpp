#include "grid.hpp"

#include "type.hpp"

#include "raylib.h"

#include <cmath>

void Grid::load() {
	create(extent);
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
			map.insert({ hex, { project(hex), 0, true } });
		}
	}
}

void Grid::render() const {
	for (auto &[point, hex] : map) {
		renderHex(point, hex);
	}
}

void Grid::renderHex(const Hex::Point& point, const Hex::State& state) const {
	DrawPoly(state.position, 6, unit.x, 0.0f, state.isEmpty ? colorHex : YELLOW);
	DrawPolyLines(state.position, 6, unit.x, 0.0f, colorLine);
	const char *pointLabel = TextFormat("(%d, %d, %d)", point.q, point.r, point.s);
	DrawText(pointLabel, state.position.x - 30.0f, state.position.y - 9.0f, 18, BLACK);
}

Hex::Point Grid::inject(Vector2 point) {
	int q0 = (point.x - origin.x) / unit.x;
	int r0 = (point.y - origin.y) / unit.y;
	Hex::Point hex = Hex::Point(q0, r0);

	float q = Hex::View.b0 * hex.q + Hex::View.b1 * hex.r;
	float r = Hex::View.b2 * hex.q + Hex::View.b3 * hex.r;
	float s = -q - r;
	// float division needs to round to the nearest int
	int q1 = static_cast<int>(roundf(q));
	int r1 = static_cast<int>(roundf(r));
	int s1 = static_cast<int>(roundf(s));
	// after rounding we do not have a guarantee that q + r + s = 0.
	// reset the component with the largest change back to what the constraint requires
	double q_diff = abs(q1 - q);
	double r_diff = abs(r1 - r);
	double s_diff = abs(s1 - s);
	if (q_diff > r_diff && q_diff > s_diff) {
		q1 = -r1 - s1;
	} else if (r_diff > s_diff) {
		r1 = -q1 - s1;
	} else {
		s1 = -q1 - r1;
	}

	return { q1, r1, s1 };
}

Vector2 Grid::project(Hex::Point point) {
	float x = (Hex::View.f0 * point.q + Hex::View.f1 * point.r) * unit.x;
	float y = (Hex::View.f2 * point.q + Hex::View.f3 * point.r) * unit.y;

	return { x + origin.x, y + origin.y };
}

void Grid::resize(int width, int height) {
	unit.x = unit.y = window.scale(HEX_SIZE);
	origin.x = window.halfWidthf;
	origin.y = window.halfHeightf;

	for (auto &[point, state] : map) {
		state.position = project(point);
	}
}