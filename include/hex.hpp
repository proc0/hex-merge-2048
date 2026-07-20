#pragma once

#include "raylib.h"

#include <array>
#include <cmath>

struct Matrix2x2Pair {
    const float f0, f1, f2, f3;
    const float b0, b1, b2, b3;
};

namespace Hex {
    struct Point { 
        int q, r, s; // cube coordinates storage
        // axial coordinates derive the third coordinate s by -q - r
        constexpr Point(int q1, int r1): q(q1), r(r1), s(-q1 - r1) {}
        constexpr Point(int q1, int r1, int s1): q(q1), r(r1), s(s1) {}
        bool operator==(const Point&) const = default;
        Point operator+(const Point& other) const {
            return Point(this->q + other.q, this->r + other.r, this->s + other.s);
        }
    };

    static inline constexpr Point Origin{ 0, 0, 0 };

    class Basis {
        // strong wrapper with implicit conversion
        // defines basis hex components of hex grid
        // private constructors and static factory methods
        explicit constexpr Basis(int q1, int r1) : q(q1), r(r1), s(-q1 - r1) {}
        explicit constexpr Basis(int q1, int r1, int s1) : q(q1), r(r1), s(s1) {}

    public:
        const int q, r, s;
        // using keyboard WASD + QE as basis names
        // starting from twelve o'clock rotating clockwise
        static constexpr Basis W() { return Basis( 0, -1,  1); }
        static constexpr Basis E() { return Basis( 1, -1,  0); }
        static constexpr Basis D() { return Basis( 1,  0, -1); }
        static constexpr Basis S() { return Basis( 0,  1, -1); }
        static constexpr Basis A() { return Basis(-1,  1,  0); }
        static constexpr Basis Q() { return Basis(-1,  0,  1); }

        // implicit conversion to Point
        operator Point() const { return Point(q, r, s); }
        // adding two basis hexes yields a non-basis point
        Point operator+(const Basis& other) const {
            return Point(this->q + other.q, this->r + other.r, this->s + other.s);
        }
    };

    enum Cardinal {
      NORTH,
      NORTH_EAST,
      SOUTH_EAST,
      SOUTH,
      SOUTH_WEST,
      NORTH_WEST
    };

    static inline constexpr Cardinal N  = Cardinal::NORTH;
    static inline constexpr Cardinal NE = Cardinal::NORTH_EAST;
    static inline constexpr Cardinal SE = Cardinal::SOUTH_EAST;
    static inline constexpr Cardinal S  = Cardinal::SOUTH;
    static inline constexpr Cardinal SW = Cardinal::SOUTH_WEST;
    static inline constexpr Cardinal NW = Cardinal::NORTH_WEST;

    // static inline const std::unordered_map<Cardinal, Cardinal> CardinalReversed = {
    //     { Cardinal::NORTH,      Cardinal::SOUTH      },
    //     { Cardinal::NORTH_EAST, Cardinal::SOUTH_WEST },
    //     { Cardinal::SOUTH_EAST, Cardinal::NORTH_WEST },
    //     { Cardinal::SOUTH,      Cardinal::NORTH      },
    //     { Cardinal::SOUTH_WEST, Cardinal::NORTH_EAST },
    //     { Cardinal::NORTH_WEST, Cardinal::SOUTH_EAST },
    // };

    static inline constexpr std::array<Basis, 6> BasisDirection = {
        Basis::W(), Basis::E(), Basis::D(), Basis::S(), Basis::A(), Basis::Q()
    };

    static inline constexpr std::array<Basis, 6> BasisReversed = {
        Basis::S(), Basis::A(), Basis::Q(), Basis::W(), Basis::E(), Basis::D()
    };
    
    static inline constexpr std::array<Basis, 6> BasisRotatedRight1 = {
        Basis::E(), Basis::D(), Basis::S(), Basis::A(), Basis::Q(), Basis::W()
    };

    static inline constexpr std::array<Basis, 6> BasisRotatedRight2 = {
        Basis::D(), Basis::S(), Basis::A(), Basis::Q(), Basis::W(), Basis::E()
    };

    static inline constexpr std::array<Basis, 6> BasisRotatedLeft1 = {
        Basis::Q(), Basis::W(), Basis::E(), Basis::D(), Basis::S(), Basis::A()
    };

    static inline constexpr std::array<Basis, 6> BasisRotatedLeft2 = {
        Basis::A(), Basis::Q(), Basis::W(), Basis::E(), Basis::D(), Basis::S()
    };
    
    // pointy top hex map
    // const Matrix2x2Pair view = Matrix2x2Pair({
    //   sqrtf(3.0f), sqrtf(3.0f)/2.0f, 0.0f, 3.0f/2.0f,
    //   sqrtf(3.0f)/3.0f, -1.0f/3.0f, 0.0, 2.0f/3.0f
    // });

    // flat top hex grid transform matrix to and from screen space
    static inline const Matrix2x2Pair View = Matrix2x2Pair({
        3.0f/2.0f, 0.0f, sqrtf(3.0f)/2.0f, sqrtf(3.0f),
        2.0f/3.0f, 0.0f, -1.0f/3.0f, sqrtf(3.0f)/3.0f
    });

    // screen point to hex grid point parameterized by grid origin and hex size
    static inline Point inject(Vector2 point, Vector2 origin, Vector2 size) {
        // undoing hex grid transform projection
        int q0 = (point.x - origin.x) / size.x;
        int r0 = (point.y - origin.y) / size.y;
        Point axial = Point(q0, r0);
        // transform matrix is 2x2, using axial system
        float q = View.b0 * axial.q + View.b1 * axial.r;
        float r = View.b2 * axial.q + View.b3 * axial.r;
        float s = -q - r;
        // float division needs to round to the nearest int
        int q1 = static_cast<int>(roundf(q));
        int r1 = static_cast<int>(roundf(r));
        int s1 = static_cast<int>(roundf(s));
        // after rounding we do not have a guarantee that q + r + s = 0.
        // reset the component with the largest change to satisfy contraint
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

    // hex grid point to screen point parameterized by grid origin and hex size
    static inline Vector2 project(Point point, Vector2 origin, Vector2 size) {
        float x = (View.f0 * point.q + View.f1 * point.r) * size.x;
        float y = (View.f2 * point.q + View.f3 * point.r) * size.y;

        return { x + origin.x, y + origin.y };
    }

    static inline Point add(Point a, Point b) {
        return Point(a.q + b.q, a.r + b.r, a.s + b.s);
    }

    static inline Point subtract(Point a, Point b) {
        return Point(a.q - b.q, a.r - b.r, a.s - b.s);
    }

    static inline Point multiply(Point a, int k) {
        return Point(a.q * k, a.r * k, a.s * k);
    }

    static inline int length(Point point) {
        return static_cast<int>((abs(point.q) + abs(point.r) + abs(point.s))*0.5f);
    }

    static inline int distance(Point a, Point b) {
        return length(subtract(a, b));
    }
}
