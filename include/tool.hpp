#pragma once

#include <cmath> // IWYU pragma: export
#include <array>
#include <algorithm>
#include <optional>
#include <span>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#define INTERSECTS(a, b) ((a.x > b.x) && (a.x < b.x + b.width) && (a.y > b.y) && (a.y < b.y + b.height))

#define ROUND4(num) (round((num) * 10000.0f)/10000.0f)

static inline int summation(int n) {
	return (n*(n+1))/2;
}

// number of frames
#define ANIMATION_LENGTH 43
#define ANIMATION_MAX_INDEX 42
// WARNING: count must equal enum items
#define ANIMATIONS_COUNT 9
namespace ANIMATION {
    enum FUNC {
        EASE_IN_QUAD,
        EASE_IN_CUBIC,
        EASE_OUT_QUAD,
        EASE_OUT_CUBIC,
        EASE_OUT_EXPO,
        EASE_OUT_ELASTIC,
        EASE_IN_OUT_CUBIC,
        EASE_IN_OUT_EXPO,
        EASE_IN_OUT_BACK,
    };
}

// JS helper function for generating animation curve values
// function generateAnimCurve(totalFrames, animFunc) { return Array.from({ length: totalFrames }, (_, i) => animFunc(i/totalFrames)).reduce((mem, val) => { return `${mem}, ${val.toPrecision(2)}f`; }) }
//
// https://easings.net
// -------------------
// function easeInQuad(x) { return x * x; }
// function easeOutQuad(x) { return 1 - (1 - x) * (1 - x); }
// function easeOutCubic(x) { return 1 - Math.pow(1 - x, 3); }
// function easeInCubic(x) { return x * x * x; }
// function easeOutElastic(x) { const c4 = (2 * Math.PI) / 3; return x === 0 ? 0 : x === 1 ? 1 : Math.pow(2, -10 * x) * Math.sin((x * 10 - 0.75) * c4) + 1; }
// function easeInOutCubic(x) { return x < 0.5 ? 4 * x * x * x : 1 - Math.pow(-2 * x + 2, 3) / 2; }

// WARNING: first value is always 0.0f!
static inline constexpr std::array<std::array<float, ANIMATION_LENGTH>, ANIMATIONS_COUNT> Animations = {{
    // ease-in quad
    { 0.0f, 0.0022f, 0.0049f, 0.0087f, 0.014f, 0.019f, 0.027f, 0.035f, 0.044f, 0.054f, 0.065f, 0.078f, 0.091f, 0.11f, 0.12f, 0.14f, 0.16f, 0.18f, 0.20f, 0.22f, 0.24f, 0.26f, 0.29f, 0.31f, 0.34f, 0.37f, 0.39f, 0.42f, 0.45f, 0.49f, 0.52f, 0.55f, 0.59f, 0.63f, 0.66f, 0.70f, 0.74f, 0.78f, 0.82f, 0.87f, 0.91f, 0.95f, 1.0f },
    // ease-in cubic
    { 0.0f, 0.00010f, 0.00034f, 0.00080f, 0.0016f, 0.0027f, 0.0043f, 0.0064f, 0.0092f, 0.013f, 0.017f, 0.022f, 0.028f, 0.035f, 0.042f, 0.052f, 0.062f, 0.073f, 0.086f, 0.10f, 0.12f, 0.13f, 0.15f, 0.17f, 0.20f, 0.22f, 0.25f, 0.28f, 0.31f, 0.34f, 0.37f, 0.41f, 0.45f, 0.49f, 0.54f, 0.59f, 0.64f, 0.69f, 0.75f, 0.80f, 0.87f, 0.93f, 1.0f },
    // ease-out quad
    { 0.0f, 0.046f, 0.091f, 0.13f, 0.18f, 0.22f, 0.26f, 0.30f, 0.34f, 0.37f, 0.41f, 0.45f, 0.48f, 0.51f, 0.55f, 0.58f, 0.61f, 0.63f, 0.66f, 0.69f, 0.71f, 0.74f, 0.76f, 0.78f, 0.80f, 0.82f, 0.84f, 0.86f, 0.88f, 0.89f, 0.91f, 0.92f, 0.93f, 0.95f, 0.96f, 0.97f, 0.97f, 0.98f, 0.99f, 0.99f, 1.0f, 1.0f, 1.0f },
    // ease-out cubic
    { 0.0f, 0.07f, 0.13f, 0.20f, 0.25f, 0.31f, 0.36f, 0.41f, 0.46f, 0.51f, 0.55f, 0.59f, 0.63f, 0.66f, 0.69f, 0.72f, 0.75f, 0.78f, 0.80f, 0.83f, 0.85f, 0.87f, 0.88f, 0.90f, 0.91f, 0.93f, 0.94f, 0.95f, 0.96f, 0.97f, 0.97f, 0.98f, 0.98f, 0.99f, 0.99f, 0.99f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    // ease-out expo
    { 0.0f, 0.15f, 0.28f, 0.38f, 0.48f, 0.55f, 0.62f, 0.68f, 0.72f, 0.77f, 0.80f, 0.83f, 0.86f, 0.88f, 0.90f, 0.91f, 0.92f, 0.94f, 0.95f, 0.95f, 0.96f, 0.97f, 0.97f, 0.98f, 0.98f, 0.98f, 0.98f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    // ease-out elastic
    { 0.0f, 0.25f, 0.59f, 0.93f, 1.2f, 1.3f, 1.4f, 1.3f, 1.2f, 1.1f, 0.97f, 0.90f, 0.87f, 0.88f, 0.91f, 0.95f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.99f, 0.99f, 0.98f, 0.98f, 0.99f, 0.99f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    // ease-in-out cubic
    { 0.0f, 0.000050f, 0.00040f, 0.0014f, 0.0032f, 0.0063f, 0.011f, 0.017f, 0.026f, 0.037f, 0.050f, 0.067f, 0.087f, 0.11f, 0.14f, 0.17f, 0.21f, 0.25f, 0.29f, 0.35f, 0.40f, 0.47f, 0.53f, 0.60f, 0.65f, 0.71f, 0.75f, 0.79f, 0.83f, 0.86f, 0.89f, 0.91f, 0.93f, 0.95f, 0.96f, 0.97f, 0.98f, 0.99f, 0.99f, 1.0f, 1.0f, 1.0f, 1.0f },
    // ease-in-out expo
    { 0.0f, 0.00067f, 0.00093f, 0.0013f, 0.0018f, 0.0024f, 0.0034f, 0.0047f, 0.0064f, 0.0089f, 0.012f, 0.017f, 0.023f, 0.032f, 0.045f, 0.062f, 0.085f, 0.12f, 0.16f, 0.22f, 0.31f, 0.43f, 0.57f, 0.69f, 0.78f, 0.84f, 0.88f, 0.92f, 0.94f, 0.96f, 0.97f, 0.98f, 0.98f, 0.99f, 0.99f, 0.99f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    // ease-in-out back
    { 0.0f, -0.0026f, -0.0098f, -0.020f, -0.033f, -0.048f, -0.062f, -0.075f, -0.087f, -0.096f, -0.10f, -0.099f, -0.092f, -0.077f, -0.054f, -0.021f, 0.022f, 0.077f, 0.15f, 0.23f, 0.32f, 0.44f, 0.56f, 0.68f, 0.77f, 0.85f, 0.92f, 0.98f, 1.0f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
}};

template<typename K, typename V, size_t Capacity>
struct StackMap {
    std::array<std::pair<K, V>, Capacity> data;
    size_t size = 0;

    StackMap() = default;

    constexpr StackMap(std::initializer_list<std::pair<K, V>> init) {
        if (init.size() > Capacity) {
            throw std::out_of_range("Initializer list exceeds StackMap capacity");
        }

        // 1. Copy from the initializer list into the array
        // We use std::move to ensure this works for complex types (strings, etc)
        std::copy(init.begin(), init.end(), data.begin());
        size = init.size();

        // 2. CRITICAL: Sort the data so that binary search (lower_bound) works
        // Manual constexpr sort (Insertion Sort is great for small arrays)
        for (size_t i = 1; i < size; ++i) {
            auto key = data[i];
            int j = i - 1;
            while (j >= 0 && data[j].first > key.first) {
                data[j + 1] = data[j];
                j--;
            }
            data[j + 1] = key;
        }
    }

    void insert(K key, V value) {
        auto it = std::lower_bound(data.begin(), data.begin() + size, key, 
            [](const auto& pair, const K& k) { return pair.first < k; });
        
        if (it != data.begin() + size && it->first == key) {
            it->second = value; // Update existing
        } else {
            // Shift elements to maintain sort (O(N), but fast for small N)
            std::move_backward(it, data.begin() + size, data.begin() + size + 1);
            *it = {key, value};
            size++;
        }
    }

    std::optional<V> get(const K& key) const {
        auto it = std::lower_bound(data.begin(), data.begin() + size, key, 
            [](const auto& pair, const K& k) { return pair.first < k; });
        
        if (it != data.begin() + size && it->first == key) return it->second;
        return std::nullopt;
    }
};

template<typename K, typename V>
struct FlatMapView {
    std::span<const std::pair<K, V>> data;

    std::optional<V> get(const K& key) const {
        auto it = std::lower_bound(data.begin(), data.end(), key, 
            [](const auto& pair, const K& k) { return pair.first < k; });
        if (it != data.end() && it->first == key) return it->second;
        return std::nullopt;
    }
};
