#pragma once

#include <pebble.h>
#include "defines.h"
#include "enums.h"
#include "player.h"
#include "../pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"

typedef struct _game Game;
struct _game {
    Player *players[MAX_PLAYERS];
    GBC_Graphics *graphics;
};

Game *Game_init(GBC_Graphics *graphics);

void Game_destroy(Game *game);

void Game_start(Game *game, char *username);
    
int Game_get_first_inactive_player_index(Game *game);
    
void Game_load_player(Game *game, char* username, int player_number, uint8_t *sprite_buffer, uint8_t *palette_buffer);
    
void Game_update_player(Game *game, char *username, int x, int y);

void Game_add_player(Game *game, char *username, int x, int y);

void Game_remove_player(Game *game, char *username);

void Game_step();

void Game_up_handler(Game *game);

void Game_down_handler(Game *game);

void Game_select_handler(Game *game);
    