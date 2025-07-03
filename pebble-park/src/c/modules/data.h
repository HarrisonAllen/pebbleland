#pragma once
#include <pebble.h>
#include "defines.h"
#include "enums.h"

typedef struct _player_data PlayerData;
struct _player_data {
    char username[USERNAME_MAX_LEN+1];
    int x;
    int y;
    Direction dir;
    int hair_style;
    int shirt_style;
    int pants_style;
    int hair_color;
    int shirt_color;
    int pants_color;
    int shoes_color;
};