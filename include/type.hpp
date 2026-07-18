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

    struct Unit {
        static constexpr Point UP         = Direction[Cardinal::NORTH];
        static constexpr Point UP_RIGHT   = Direction[Cardinal::NORTH_EAST];
        static constexpr Point DOWN_RIGHT = Direction[Cardinal::SOUTH_EAST];
        static constexpr Point DOWN       = Direction[Cardinal::SOUTH];
        static constexpr Point DOWN_LEFT  = Direction[Cardinal::SOUTH_WEST];
        static constexpr Point UP_LEFT    = Direction[Cardinal::NORTH_WEST];
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
        Point({  0, -1,  1 }),
        Point({  1, -1,  0 }),
        Point({  1,  0, -1 }),
        Point({  0,  1, -1 }),
        Point({ -1,  1,  0 }),
        Point({ -1,  0,  1 })
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

    // flat top hex map
    static inline const Matrix2x2Pair View = Matrix2x2Pair({
        3.0f/2.0f, 0.0f, sqrtf(3.0f)/2.0f, sqrtf(3.0f),
        2.0f/3.0f, 0.0f, -1.0f/3.0f, sqrtf(3.0f)/3.0f
    });

    struct State {
        Vector2 position;
        int key;
    };

    // pointy top hex map
    // const Matrix2x2Pair view = Matrix2x2Pair({
    //   sqrtf(3.0f), sqrtf(3.0f)/2.0f, 0.0f, 3.0f/2.0f,
    //   sqrtf(3.0f)/3.0f, -1.0f/3.0f, 0.0, 2.0f/3.0f
    // });
}


// struct Unit {
//     static constexpr Hex::Point UP         = Hex::Direction[Hex::Cardinal::NORTH];
//     static constexpr Hex::Point UP_RIGHT   = Hex::Direction[Hex::Cardinal::NORTH_EAST];
//     static constexpr Hex::Point DOWN_RIGHT = Hex::Direction[Hex::Cardinal::SOUTH_EAST];
//     static constexpr Hex::Point DOWN       = Hex::Direction[Hex::Cardinal::SOUTH];
//     static constexpr Hex::Point DOWN_LEFT  = Hex::Direction[Hex::Cardinal::SOUTH_WEST];
//     static constexpr Hex::Point UP_LEFT    = Hex::Direction[Hex::Cardinal::NORTH_WEST];
// };

struct ChipState {
    int value;
};
