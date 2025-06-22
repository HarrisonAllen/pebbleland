#pragma once

#include <pebble.h>
#include "defines.h"
#include "enums.h"
#include "player.h"
#include "settings.h"
#include "../pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"
#include "windows/slide_layer.h"
#include "background.h"
#include "messages/message_queue.h"

typedef struct _game Game;
struct _game {
    Player *players[MAX_PLAYERS];
    Player *player_one;
    GBC_Graphics *graphics;
    Window *window;
    ClaySettings *settings;
    int accel_cal_x, accel_cal_y;
    bool in_focus, paused;
    GBitmap *icon_up, *icon_middle, *icon_down;
    SlideLayer *notification;
    Background *background;
    MessageQueue *message_queue;
    Layer *sprite_layer;
};

Game *Game_init(GBC_Graphics *graphics, Window *window, ClaySettings *settings);

void Game_destroy(Game *game);

void Game_start(Game *game);
    
int Game_get_first_inactive_player_index(Game *game);
    
void Game_load_player(Game *game, char* username, int player_number, int hair, int shirt, int pants, uint8_t *colors);
    
void Game_update_player(Game *game, char *username, int x, int y);

void Game_add_player(Game *game, char *username, int x, int y);

void Game_remove_player(Game *game, char *username);

void Game_step(Game *game);

void Game_up_handler(Game *game);

void Game_down_handler(Game *game);

void Game_select_handler(Game *game);

void Game_back_handler(Game *game);

void Game_focus_handler(Game *game, bool in_focus);

void Game_show_notification(Game *game, char *text);

void Game_queue_notification(Game *game, char *text);