#pragma once

#include <pebble.h>
#include "defines.h"
#include "enums.h"
#include "../pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"

typedef struct _player Player;
struct _player {
    bool active;
    GBC_Graphics *graphics;
    int x;
    int y;
    Direction direction;
    int number;
    int tile_offset;
    int num_tiles;
    uint8_t palette[GBC_PALETTE_NUM_BYTES];
    char username[USERNAME_MAX_LEN+1];
};

// Set up basic player info
Player *Player_initialize(GBC_Graphics *graphics, int number);

// Destroy the player object
void Player_destroy(Player *player);

// Make player invisible
void Player_hide(Player *player);

// Make player visible
void Player_show(Player *player);

// Hide player and make inactive
void Player_deactivate(Player *player);

// Show player and make active
void Player_activate(Player *player);

// Load username for player
void Player_set_username(Player *player, char *username);

// Set location of player and update graphics
void Player_set_position(Player *player, int x, int y);

// Move player by x, y
void Player_move(Player *player, int x, int y);

// Load in a sprite and palette from a buffer
void Player_load_sprite_and_palette(Player *player, uint8_t *sprite_data, uint8_t *palette_data);

// Update the sprite pretty much
void Player_render(Player *player);

void Player_set_direction(Player *player, Direction direction);

void Player_rotate_clockwise(Player *player);

void Player_rotate_counterclockwise(Player *player);

void Player_take_step(Player *player);