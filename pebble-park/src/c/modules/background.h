#pragma once
#include <pebble.h>
#include "../pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"
#include "enums.h"

typedef struct _background Background;
struct _background {
    GBC_Graphics *graphics;
    uint8_t *tilemap;
};

void Background_load_tiles(Background *background, uint8_t bg_root_x, uint8_t bg_root_y, int tile_root_x, int tile_root_y, uint8_t num_x_tiles, uint8_t num_y_tiles);

void Background_load_screen(Background *background, int player_x, int player_y);

void Background_load_tiles_in_direction(Background *background, Direction direction, int player_x, int player_y);

void Background_load_resources(Background *background);

Background *Background_init(GBC_Graphics *graphics);

void Background_destroy(Background *background);
    
void Background_move(Background *background, int x, int y);