#pragma once

#include <pebble.h>

typedef enum {
    S_LOGIN,
    S_MAIN,
    S_PLAY
} AppState;

typedef enum {
    D_DOWN,
    D_LEFT,
    D_UP,
    D_RIGHT,
    D_MAX
} Direction;

typedef enum {
    G_UNSTARTED,
    G_MAIN,
} GameState;

typedef enum {
    W_STAND,
    W_STEP,
    W_WALK
} WalkState;

typedef enum {
    P_STAND,
    P_WALK,
    P_TALK
} PlayerState;

typedef enum {
    Q_NONE, // 0
    Q_UP, // 1
    Q_DOWN, // 2
    Q_BACK, // 3
    Q_SELECT, // 4
    Q_SELECT_DOUBLE, // 5
    Q_SELECT_HOLD, // 6
    Q_SELECT_PRESS, // 7
    Q_SELECT_RELEASE // 8
} QueuedInput;

typedef enum {
    M_INFO,
    M_USERS,
    M_SETTINGS,
    M_LOGOUT
} MenuItems;