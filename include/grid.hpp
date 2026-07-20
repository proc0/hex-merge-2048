#pragma once

#include "window.hpp"
#include "type.hpp"
#include "hex.hpp"

#include <unordered_map>

// https://www.redblobgames.com/grids/hexagons/implementation.html
namespace std {
    template <> struct hash<Hex::Point> {
        size_t operator()(const Hex::Point& h) const {
            hash<int> int_hash;
            size_t hq = int_hash(h.q);
            size_t hr = int_hash(h.r);
            return hq ^ (hr + 0x9e3779b9 + (hq << 6) + (hq >> 2));
        }
    };
}

class Grid : public Layer {
    RenderTexture2D target;
    Rectangle targetSource;
    Rectangle targetDestination;

	std::unordered_map<Hex::Point, HexState> map;

	const Window& window;
	Vector2 origin = { window.halfWidthf, window.halfHeightf };

	Color colorHex = BEIGE;
	Color colorLine = DARKGRAY;

	Vector2 unit = { HEX_SIZE, HEX_SIZE };
	int extent = GRID_EXTENT;

public:
	Grid(const Window& window): window(window) {}
	~Grid() = default;

	void load();
	void loadTarget();
	void create(int extent);
	void reset();

	void render() const;
	void renderGrid() const;
	void renderHex(const Hex::Point&, const HexState&) const;

	HexState getState(Hex::Point) const;
	Vector2 getPosition(Hex::Point) const;
	Vector2 getUnit() const;

	void place(Hex::Point, int value);
	void clear(Hex::Point point);

    Hex::Point walk(Hex::Basis, Hex::Point) const;
    Hex::Point corner(Hex::Point) const;
    Hex::Point findAny() const;
    Hex::Point findCenter() const;
    Hex::Point findRandom() const;

    bool walkEdge(Hex::Basis dir, Hex::Point hex) const;
    bool vacant(Hex::Point) const;
    bool occupied(Hex::Point) const;
    bool inside(Hex::Point point) const;
    bool filled() const;
    
    int size() const;
    void resize(int width, int height) override;
    void unload();
};
