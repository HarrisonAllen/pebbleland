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