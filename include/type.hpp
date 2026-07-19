#pragma once

#include "timer.hpp"

#include "raylib.h"

#include <array>
#include <cmath>

namespace Action {
    enum Surface {
        DO_NOTHING,
        NEW_GAME,
        CONFIRM_TUTORIAL,
        MOVE_UP,
        MOVE_RIGHT,
        MOVE_DOWN,
        MOVE_LEFT,
        LOAD_GAME,
        RESTART,
        SAVE_GAME,
        OPTIONS,
        RESUME_GAME,
        MAIN_MENU,
        CONFIRM_RETURN,
        CANCEL_RETURN,
        CONFIRM_OPTIONS,
        CANCEL_OPTIONS,
        CHANGE_OPTIONS_GAME,
        CHANGE_OPTIONS_AUDIO,
        CHANGE_OPTIONS_INPUTS,
        QUIT_APP,
    };
}

namespace Event {
    enum Input {
        IDLE,
        PRIMARY,
        PRIMARY_DOWN,
        PRIMARY_UP,
        SECONDARY,
        SECONDARY_DOWN,
        SECONDARY_UP,
        TERTIARY,
        TERTIARY_DOWN,
        TERTIARY_UP,
        MOVE_UP,
        MOVE_RIGHT,
        MOVE_DOWN,
        MOVE_LEFT,
        ZOOM_IN,
        ZOOM_OUT,
        SWIPE_UP,
        SWIPE_DOWN,
        SWIPE_RIGHT,
        SWIPE_LEFT,
        KEY_ESCAPE,
        KEY_OTHER
    };

    enum Timer {
        READY,
        RUNNING,
        FINISHED
    };

    enum Surface {
        NO_EVENT,
        SHOW_TUTORIAL,
        CONFIRM_TUTORIAL,
        SHOW_RETURN_MAIN_MENU_CONFIRMATION,
        SHOW_OPTIONS,
    };
}

namespace State {
    enum App {
        LOAD,
        RUN,
        HOLD,
        HALT
    };

    enum Screen {
        INTRO,
        TITLE,
        MAIN,
        GAME
    };

    enum Game {
        START,
        PLAY,
        PAUSE,
        OVER,
        WIN,
        FINISH
    };

    enum Pointer {
        IDLE,
        GRAB,
        DRAG,
        DROP
    };
}

struct InputEvent {
    Event::Input id;
    Vector2 position;
    Vector2 mouseWheel;
};

#define DEFAULT_INPUT InputEvent({ .id = Event::Input::IDLE, .position = Vector2({}) })

struct GameState {
    int score;
    State::Game state;
    TimerId totalTimeId;
};

struct WorldState {
    bool reachedGoal;
    bool failedGoal;
};

class Layer {
public:
    virtual ~Layer() = default;
    virtual void resize(int width, int height) {}
};

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
    };

    enum Cardinal {
      NORTH,
      NORTH_EAST,
      SOUTH_EAST,
      SOUTH,
      SOUTH_WEST,
      NORTH_WEST
    };

    static inline constexpr std::array<Point, 6> Direction = { 
        Point({  0, -1,  1 }),
        Point({  1, -1,  0 }),
        Point({  1,  0, -1 }),
        Point({  0,  1, -1 }),
        Point({ -1,  1,  0 }),
        Point({ -1,  0,  1 })
    };

    static inline constexpr Cardinal UP   = Cardinal::NORTH;
    static inline constexpr Cardinal UP_R = Cardinal::NORTH_EAST;
    static inline constexpr Cardinal DN_R = Cardinal::SOUTH_EAST;
    static inline constexpr Cardinal DN   = Cardinal::SOUTH;
    static inline constexpr Cardinal DN_L = Cardinal::SOUTH_WEST;
    static inline constexpr Cardinal UP_L = Cardinal::NORTH_WEST;

    struct Unit {   
        static constexpr Point UP   = Direction[Cardinal::NORTH];
        static constexpr Point UP_R = Direction[Cardinal::NORTH_EAST];
        static constexpr Point DN_R = Direction[Cardinal::SOUTH_EAST];
        static constexpr Point DN   = Direction[Cardinal::SOUTH];
        static constexpr Point DN_L = Direction[Cardinal::SOUTH_WEST];
        static constexpr Point UP_L = Direction[Cardinal::NORTH_WEST];
    };

    static inline constexpr std::array<Point, 6> Reverse = { 
        Point({  0,  1, -1 }),
        Point({ -1,  1,  0 }),
        Point({ -1,  0,  1 }),
        Point({  0, -1,  1 }),
        Point({  1, -1,  0 }),
        Point({  1,  0, -1 })
    };

    static inline constexpr std::array<Point, 6> RotateClockwise1 = { 
        Point({  1, -1,  0 }),
        Point({  1,  0, -1 }),
        Point({  0,  1, -1 }),
        Point({ -1,  1,  0 }),
        Point({ -1,  0,  1 }),
        Point({  0, -1,  1 })
    };

    static inline constexpr std::array<Point, 6> RotateClockwise2 = { 
        Point({  1,  0, -1 }),
        Point({  0,  1, -1 }),
        Point({ -1,  1,  0 }),
        Point({ -1,  0,  1 }),
        Point({  0, -1,  1 }),
        Point({  1, -1,  0 })
    };

    static inline constexpr std::array<Point, 6> RotateCounterwise1 = { 
        Point({ -1,  0,  1 }),
        Point({  0, -1,  1 }),
        Point({  1, -1,  0 }),
        Point({  1,  0, -1 }),
        Point({  0,  1, -1 }),
        Point({ -1,  1,  0 })
    };

    static inline constexpr std::array<Point, 6> RotateCounterwise2 = { 
        Point({ -1,  1,  0 }),
        Point({ -1,  0,  1 }),
        Point({  0, -1,  1 }),
        Point({  1, -1,  0 }),
        Point({  1,  0, -1 }),
        Point({  0,  1, -1 })
    };

    // pointy top hex map
    // const Matrix2x2Pair view = Matrix2x2Pair({
    //   sqrtf(3.0f), sqrtf(3.0f)/2.0f, 0.0f, 3.0f/2.0f,
    //   sqrtf(3.0f)/3.0f, -1.0f/3.0f, 0.0, 2.0f/3.0f
    // });

    // flat top hex map
    static inline const Matrix2x2Pair View = Matrix2x2Pair({
        3.0f/2.0f, 0.0f, sqrtf(3.0f)/2.0f, sqrtf(3.0f),
        2.0f/3.0f, 0.0f, -1.0f/3.0f, sqrtf(3.0f)/3.0f
    });

    static inline Point inject(Vector2 point, Vector2 origin, Vector2 unit) {
        int q0 = (point.x - origin.x) / unit.x;
        int r0 = (point.y - origin.y) / unit.y;
        Point hex = Point(q0, r0);

        float q = View.b0 * hex.q + View.b1 * hex.r;
        float r = View.b2 * hex.q + View.b3 * hex.r;
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

    static inline Vector2 project(Point point, Vector2 origin, Vector2 unit) {
        float x = (View.f0 * point.q + View.f1 * point.r) * unit.x;
        float y = (View.f2 * point.q + View.f3 * point.r) * unit.y;

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

// #define DIR_UP   Hex::Cardinal::NORTH
// #define DIR_UP_R Hex::Cardinal::NORTH_EAST
// #define DIR_DN_R Hex::Cardinal::SOUTH_EAST
// #define DIR_DN   Hex::Cardinal::SOUTH
// #define DIR_DN_L Hex::Cardinal::SOUTH_WEST
// #define DIR_UP_L Hex::Cardinal::NORTH_WEST

struct HexState {
    Vector2 position;
    int key;
};

