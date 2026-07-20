#pragma once

#include <cmath> // IWYU pragma: export
#include <array>

#define INTERSECTS(a, b) ((a.x > b.x) && (a.x < b.x + b.width) && (a.y > b.y) && (a.y < b.y + b.height))

#define ROUND4(num) (round((num) * 10000.0f)/10000.0f)

static inline int summation(int n) {
	return (n*(n+1))/2;
}

static constexpr std::array<float, 42> ANIM_EASE_OUT_QUAD = { 1.0f, 0.50f, 0.11f, 0.036f, 0.015f, 0.0079f, 0.0046f, 0.0029f, 0.0019f, 0.0014f, 0.0010f, 0.00075f, 0.00058f, 0.00045f, 0.00036f, 0.00030f, 0.00024f, 0.00020f, 0.00017f, 0.00015f, 0.00012f, 0.00011f, 0.000094f, 0.000082f, 0.000072f, 0.000064f, 0.000057f, 0.000051f, 0.000046f, 0.000041f, 0.000037f, 0.000034f, 0.000031f, 0.000028f, 0.000025f, 0.000023f, 0.000021f, 0.000020f, 0.000018f, 0.000017f, 0.000016f, 0.000015f };
static constexpr std::array<float, 42> ANIM_FRAMES = { 0.000054f, 0.00043f, 0.0015f, 0.0035f, 0.0067f, 0.012f, 0.019f, 0.028f, 0.039f, 0.054f, 0.072f, 0.093f, 0.12f, 0.15f, 0.18f, 0.22f, 0.27f, 0.31f, 0.37f, 0.43f, 0.50f, 0.57f, 0.63f, 0.69f, 0.73f, 0.78f, 0.82f, 0.85f, 0.88f, 0.91f, 0.93f, 0.95f, 0.96f, 0.97f, 0.98f, 0.99f, 0.99f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
