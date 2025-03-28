#pragma once

#include <pebble.h>
#include "defines.h"
#include "enums.h"
#include "../pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"
#include "settings.h"

typedef struct _player Player;
struct _player {
    GBC_Graphics *graphics;
    ClaySettings *settings;
    bool active;
    int x;
    int y;
    Direction direction;
    Direction tilt_direction;
    int number;
    int sprite_number;
    int tile_offset;
    int num_tiles;
    uint8_t palette[GBC_PALETTE_NUM_BYTES];
    char username[USERNAME_MAX_LEN+1];
    PlayerState state;
    WalkState walk_state;
    int walk_frame;
    QueuedInput queued_input;
    bool select_pressed;
};

// Set up basic player info
Player *Player_initialize(int number, GBC_Graphics *graphics, ClaySettings *settings);

// Destroy the player object
void Player_destroy(Player *player);

// Queue an input for the player
void Player_push_input(Player *player, QueuedInput input);

// Pop an input from the queue
void Player_pop_input(Player *player);

// main loop
void Player_step(Player *player);

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

void Player_set_tilt_direction(Player *player, Direction tilt_direction);

void Player_take_step(Player *player);