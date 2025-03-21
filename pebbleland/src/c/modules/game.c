#include "game.h"
#include "palettes.h"
#include "communication.h"

Game *Game_init(GBC_Graphics *graphics) {
    Game *game = NULL;
    game = malloc(sizeof(Game));
    if (game == NULL)
        return NULL;
    game->graphics = graphics;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        game->players[i] = Player_initialize(graphics, i);
    }
    return game;
}

void Game_destroy(Game *game) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Player_destroy(game->players[i]);
    }
    if (game != NULL) {
        free(game);
    }
}

void Game_start(Game *game, char *username) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "GAme started");
    // Load basic map
    GBC_Graphics_set_bg_palette_array(game->graphics, 0, BLANK_BG_PALETTE);

    // Load player one?
    // TODO: load these in via args instead of hardcoded like this
    ResHandle tilesheet_handle = resource_get_handle(RESOURCE_ID_PLAYER_TILESHEET);
    size_t res_size = resource_size(tilesheet_handle);
    uint8_t *sprite_data = (uint8_t*)malloc(res_size);
    resource_load(tilesheet_handle, sprite_data, res_size);
    Game_load_player(game, username, 0, sprite_data, PLAYER_ONE_PALETTE);
    // Player_load_sprite_and_palette(game->players[0], sprite_data, PLAYER_ONE_PALETTE);
    free(sprite_data);
    int player_x = ((GBC_Graphics_get_screen_width(game->graphics) / 2 - (PLAYER_SPRITE_WIDTH / 2)) / 8) * 8;
    int player_y = ((GBC_Graphics_get_screen_height(game->graphics) / 2 - (PLAYER_SPRITE_HEIGHT / 2)) / 8) * 8;
    Player_set_position(game->players[0], player_x, player_y);

    // Load other players
    broadcast_connect(player_x, player_y, true);
    
    // Show game (enable lcdc bit)
    GBC_Graphics_lcdc_set_enabled(game->graphics, true);
    GBC_Graphics_render(game->graphics);
}

int Game_get_first_inactive_player_index(Game *game) {
    for (int i = 1; i < MAX_PLAYERS; i++) {
        if (!game->players[i]->active) {
            return i;
        }
    }
    return -1;
}

int Game_get_player_by_name(Game *game, char* username) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i]->active && strcmp(game->players[i]->username, username) == 0) {
            return i;
        }
    }
    return -1;
}

void Game_load_player(Game *game, char* username, int player_number, uint8_t *sprite_buffer, uint8_t *palette_buffer) {
    if (player_number == -1) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Invalid player slot %d. Cannot add user: %s", player_number, username);
    } else {
        Player *player = game->players[player_number];
        Player_set_username(player, username);
        Player_load_sprite_and_palette(player, sprite_buffer, palette_buffer);
        Player_activate(player);
    }
    GBC_Graphics_render(game->graphics);
}

void Game_update_player(Game *game, char *username, int x, int y) {
    int player_number = Game_get_player_by_name(game, username);
    if (player_number == -1) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "User not found: %s", username);
        poll_users();
    } else if (player_number != 0) {
        Player *player = game->players[player_number];
        if (!player->active) {
            Player_activate(player);
        }
        Player_set_position(player, x, y);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "After update for %s (user %d): %s @ (%d, %d)", username, player_number, player->active ? "active" : "inactive", player->x, player->y);
    }
    GBC_Graphics_render(game->graphics);
}

void Game_add_player(Game *game, char *username, int x, int y) {
    int player_number = Game_get_player_by_name(game, username);
    if (player_number == -1 || !game->players[player_number]->active) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "New user: %s", username);
        int new_player_number = Game_get_first_inactive_player_index(game);
        if (new_player_number == -1) {
            APP_LOG(APP_LOG_LEVEL_WARNING, "Player array full! Cannot add user: %s", username);
        } else {
            // TODO: replace with actual bytes from server
            ResHandle tilesheet_handle = resource_get_handle(RESOURCE_ID_DEFAULT_TILESHEET);
            size_t res_size = resource_size(tilesheet_handle);
            uint8_t *sprite_data = (uint8_t*)malloc(res_size);
            resource_load(tilesheet_handle, sprite_data, res_size);
            Game_load_player(game, username, new_player_number, sprite_data, DEFAULT_PALETTE);
            free(sprite_data);
            Game_update_player(game, username, x, y);
        }
    } else {
        APP_LOG(APP_LOG_LEVEL_WARNING, "User already active: %s", username);
    }
}

void Game_remove_player(Game *game, char *username) {
    int player_number = Game_get_player_by_name(game, username);
    if (player_number != -1 && player_number != 0) {
        Player_deactivate(game->players[player_number]);
    }
}

void Game_step() {
    // TODO: add game logic
}

void Game_up_handler(Game *game) {
    Player_rotate_counterclockwise(game->players[0]);
    GBC_Graphics_render(game->graphics);
}

void Game_down_handler(Game *game) {
    Player_rotate_clockwise(game->players[0]);
    GBC_Graphics_render(game->graphics);
}

void Game_select_handler(Game *game) {
    Player_take_step(game->players[0]);
    broadcast_position(game->players[0]->x, game->players[0]->y);
    GBC_Graphics_render(game->graphics);
}