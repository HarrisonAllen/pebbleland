#pragma once

#include <pebble.h>
#include "defines.h"
#include "enums.h"
#include "../pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"
#include "settings.h"
#include "background.h"

typedef struct _player Player;
struct _player {
    GBC_Graphics *graphics;
    Background *background;
    ClaySettings *settings;
    Player *player_one;
    Window *window;
    TextLayer *text_layer;
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

Player *Player_initialize(int number, GBC_Graphics *graphics, Background *background, ClaySettings *settings, Player *player_one, Window *window);

void Player_destroy(Player *player);

void Player_push_input(Player *player, QueuedInput input);

void Player_pop_input(Player *player);

void Player_step(Player *player);

void Player_hide(Player *player);

void Player_show(Player *player);

void Player_deactivate(Player *player);

void Player_set_up_username_text(Player *player);

void Player_activate(Player *player);

void Player_set_username(Player *player, char *username);

void Player_set_position(Player *player, int x, int y);

void Player_move(Player *player, int x, int y);

void Player_load_sprite_and_palette(Player *player, int hairdo, int clothes, uint8_t *palette_data);

void Player_render_username(Player *player);

void Player_render(Player *player);

void Player_set_direction(Player *player, Direction direction);

void Player_rotate_clockwise(Player *player);

void Player_rotate_counterclockwise(Player *player);

void Player_set_tilt_direction(Player *player, Direction tilt_direction);

void Player_take_step(Player *player);
