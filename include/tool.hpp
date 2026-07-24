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
