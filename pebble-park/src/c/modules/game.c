#include "game.h"
#include "palettes/sprite_palettes.h"
#include "communication.h"
#include "menus/main_menu.h"
#include "windows/slide_layer.h"
#include "sprites/player_sprites.h"

Game *Game_init(GBC_Graphics *graphics, Window *window, ClaySettings *settings) {
    Game *game = NULL;
    game = malloc(sizeof(Game));
    if (game == NULL)
        return NULL;
    game->graphics = graphics;
    game->window = window;
    game->settings = settings;
    game->background = Background_init(graphics);
    game->players[0] = Player_initialize(0, graphics, game->background, settings, NULL, window);
    game->player_one = game->players[0];
    for (int i = 1; i < MAX_PLAYERS; i++) {
        game->players[i] = Player_initialize(i, graphics, game->background, settings, game->player_one, window);
    }
    game->message_queue = MessageQueue_init();
    return game;
}

void Game_destroy(Game *game) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Player_destroy(game->players[i]);
    }
    if (game->notification != NULL) {
        SlideLayer_cancel(game->notification);
    }
    if (game->icon_up != NULL) 
        gbitmap_destroy(game->icon_up);
    if (game->icon_middle != NULL) 
        gbitmap_destroy(game->icon_middle);
    if (game->icon_down != NULL) 
        gbitmap_destroy(game->icon_down);
    if (game->graphics != NULL)
        Background_destroy(game->background);
    if (game->message_queue != NULL)
        MessageQueue_destroy(game->message_queue);
        
    if (game != NULL) {
        free(game);
    }
}

void Game_start(Game *game) {
    // Load basic map
    GBC_Graphics_set_bg_palette_array(game->graphics, 0, BLANK_BG_PALETTE);
    window_set_background_color(game->window, GColorBlack);

    // Load sprites
    GBC_Graphics_load_entire_tilesheet_into_vram(game->graphics, HAIRDO_TILESHEET, HAIRDO_VRAM_START, PLAYER_VRAM);
    GBC_Graphics_load_entire_tilesheet_into_vram(game->graphics, CLOTHES_TILESHEET, CLOTHES_VRAM_START, PLAYER_VRAM);

    // TODO: load in hairdo, clothes, colors from web
    Game_load_player(game, game->settings->Username, 0, rand()%HAIRDO_COUNT, rand()%CLOTHES_COUNT, DEFAULT_SPRITE_PALETTE);
    int player_x = 0; // ((GBC_Graphics_get_screen_width(game->graphics) / 2 - (PLAYER_SPRITE_WIDTH / 2)) / 8) * 8;
    int player_y = 0; // ((GBC_Graphics_get_screen_height(game->graphics) / 2 - (PLAYER_SPRITE_HEIGHT / 2)) / 8) * 8;
    Player_set_position(game->player_one, player_x, player_y);

    game->in_focus = true;

    // Load other players
    broadcast_connect(player_x, player_y, true);

    // Set up background
    Background_load_resources(game->background);
    Background_load_screen(game->background, game->player_one->x, game->player_one->y);
    
    // Show game (enable lcdc bit)
    GBC_Graphics_lcdc_set_enabled(game->graphics, true);
    GBC_Graphics_render(game->graphics);

    // And show welcome message
    char welcome_text[POPUP_TEXT_LEN];
    snprintf(welcome_text, POPUP_TEXT_LEN, "Welcome, %s!", game->settings->Username);
    Game_queue_notification(game, welcome_text);
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

void Game_load_player(Game *game, char* username, int player_number, int hairdo, int clothes, uint8_t *palette_buffer) {
    if (player_number == -1) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Invalid player slot %d. Cannot add user: %s", player_number, username);
    } else {
        Player *player = game->players[player_number];
        Player_set_username(player, username);
        Player_load_sprite_and_palette(player, hairdo, clothes, palette_buffer);
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
            // TODO: replace with sprite indices from server
            Game_load_player(game, username, new_player_number, rand()%HAIRDO_COUNT, rand()%CLOTHES_COUNT, DEFAULT_SPRITE_PALETTE);
            Game_update_player(game, username, x, y);

            char connect_message[40];
            snprintf(connect_message, 40, "%s connected", username);
            Game_queue_notification(game, connect_message);
        }
    } else {
        APP_LOG(APP_LOG_LEVEL_WARNING, "User already active: %s", username);
    }
}

void Game_remove_player(Game *game, char *username) {
    int player_number = Game_get_player_by_name(game, username);
    if (player_number != -1 && player_number != 0) {
        Player_deactivate(game->players[player_number]);

        char disconnect_message[40];
        snprintf(disconnect_message, 40, "%s disconnected", username);
        Game_queue_notification(game, disconnect_message);
    }
}

Direction Game_calculate_tilt(Game *game) {
    AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
    accel_service_peek(&accel);
    int accel_x = accel.x - game->accel_cal_x;
    int accel_y = accel.y - game->accel_cal_y;
    if (abs(accel_x) > abs(accel_y)) {
        if (accel_x < -TILT_THRESHOLD) {
            return D_LEFT;
        } else if (accel_x > TILT_THRESHOLD) {
            return D_RIGHT;
        }
    } else {
        if (accel_y < -TILT_THRESHOLD) {
            return D_DOWN;
        } else if (accel_y > TILT_THRESHOLD) {
            return D_UP;
        }
    }
    return D_MAX;
}

void Game_calibrate_accel(Game *game) {
    AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
    accel_service_peek(&accel);
    game->accel_cal_x = accel.x;
    game->accel_cal_y = accel.y;
    Player_set_tilt_direction(game->player_one, D_MAX);
}

void Game_step(Game *game) {
    // TODO: add game logic
    // TODO: don't run game logic when not in focus
    if (window_stack_get_top_window() == game->window && game->in_focus) {
        if (game->paused) {
            Game_calibrate_accel(game);
            game->paused = false;
        }
    } else {
        if (!game->paused) {
            game->paused = true;
        }
    }
    if (!game->paused) {
        if (game->notification == NULL && !MessageQueue_is_empty(game->message_queue)) {
            Game_show_notification(game, MessageQueue_pop(game->message_queue));
        }
        if (game->settings->Tilt) {
            Player_set_tilt_direction(game->player_one, Game_calculate_tilt(game));
        }
        Player_step(game->player_one);
        for (int i = 1; i < MAX_PLAYERS; i++) {
            Player_render(game->players[i]);
        }
        GBC_Graphics_render(game->graphics);
    }
}

void Game_up_handler(Game *game) {
    // Turn left
    Player_push_input(game->player_one, Q_UP);
}

void Game_down_handler(Game *game) {
    // Turn right
    Player_push_input(game->player_one, Q_DOWN);
}

void Game_select_handler(Game *game) {
    // Step or interact or stop walk
    Player_push_input(game->player_one, Q_SELECT);
}

void Game_back_handler(Game *game) {
    // Open menu
    if (game->notification != NULL) {
        SlideLayer_cancel(game->notification);
    } else {
        MainMenu_init(game->players, game->settings);
    }
}

void Game_focus_handler(Game *game, bool in_focus) {
    game->in_focus = in_focus;
}

void Game_release_notification(void *context) {
    ((Game *)context)->notification = NULL;
}

void Game_show_notification(Game *game, char *text) {
    if (game->notification != NULL) {
        SlideLayer_cancel(game->notification);
    }
    game->notification = SlideLayer_init(game->window, text, Game_release_notification, (void *) game);
}

void Game_queue_notification(Game *game, char *text) {
    if (!MessageQueue_push(game->message_queue, text)) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Message queue full, notification not queued");
    }
}

// New logic:
// * Queue notification = add to queue
// * On step, if game->notification == NULL, pop queue and show notification
// * Release is same