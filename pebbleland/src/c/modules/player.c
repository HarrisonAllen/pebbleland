#include "player.h"
#include "communication.h"

Player *Player_initialize(GBC_Graphics *graphics, int number) {
    Player *player = NULL;
    player = malloc(sizeof(Player));
    if (player == NULL)
        return NULL;
    player->active = false;
    player->graphics = graphics;
    player->number = number;
    player->sprite_number = MAX_PLAYERS - (player->number + 1);
    player->direction = D_DOWN;
    if (player->number == 0) {
        player->tile_offset = 0;
        player->num_tiles = PLAYER_ONE_NUM_TILES;
    } else {
        player->tile_offset = PLAYER_ONE_NUM_TILES + (player->number - 1) * PLAYER_SPRITE_NUM_TILES;
        player->num_tiles = PLAYER_SPRITE_NUM_TILES;
    }
    GBC_Graphics_oam_set_sprite(player->graphics, player->sprite_number, 0, 0, player->tile_offset, GBC_Graphics_attr_make(player->sprite_number, PLAYER_VRAM, false, false, true), PLAYER_SPRITE_TILE_WIDTH - 1, PLAYER_SPRITE_TILE_HEIGHT - 1, 0, 0);
    Player_render(player);
    return player;
}

void Player_destroy(Player *player) {
    if (player != NULL) {
        free(player);
    }
}

void Player_hide(Player *player) {
    GBC_Graphics_oam_set_sprite_hidden(player->graphics, player->sprite_number, true);
}

void Player_show(Player *player) {
    GBC_Graphics_oam_set_sprite_hidden(player->graphics, player->sprite_number, false);
}

void Player_deactivate(Player *player) {
    player->active = false;
    Player_hide(player);
}

void Player_activate(Player *player) {
    player->active = true;
    Player_show(player);
}

void Player_set_username(Player *player, char *username) {
    strcpy(player->username, username);
}

void Player_set_position(Player *player, int x, int y) {
    player->x = x;
    player->y = y;
    GBC_Graphics_oam_set_sprite_pos(player->graphics, player->sprite_number, player->x + GBC_SPRITE_OFFSET_X, player->y + GBC_SPRITE_OFFSET_Y);
}

void Player_move(Player *player, int x, int y) {
    Player_set_position(player, player->x + x, player->y + y);
}

void Player_load_sprite_and_palette(Player *player, uint8_t *sprite_data, uint8_t *palette_data) {
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Loading %d tiles for player %d", player->num_tiles, player->number);
    GBC_Graphics_load_from_buffer_into_vram(player->graphics, sprite_data, player->num_tiles, player->tile_offset, PLAYER_VRAM);
    GBC_Graphics_set_sprite_palette_array(player->graphics, player->sprite_number, palette_data);
}

void Player_render(Player *player) {
    if (player->number == 0) {
        GBC_Graphics_oam_set_sprite_tile(player->graphics, player->sprite_number, player->tile_offset + player->direction * PLAYER_SPRITE_NUM_TILES);
    } else {
        GBC_Graphics_oam_set_sprite_tile(player->graphics, player->sprite_number, player->tile_offset);
    }
}

void Player_set_direction(Player *player, Direction direction) {
    player->direction = direction;
    Player_render(player);
}

void Player_rotate_clockwise(Player *player) {
    Player_set_direction(player, (player->direction + 1 + D_MAX) % D_MAX);
}

void Player_rotate_counterclockwise(Player *player) {
    Player_set_direction(player, (player->direction - 1 + D_MAX) % D_MAX); // The + D_MAX is to avoid negative mod
}

void Player_take_step(Player *player) {
    switch (player->direction) {
        case D_UP:
            Player_move(player, 0, -PLAYER_STEP_SIZE);
            break;
        case D_DOWN:
            Player_move(player, 0, PLAYER_STEP_SIZE);
            break;
        case D_LEFT:
            Player_move(player, -PLAYER_STEP_SIZE, 0);
            break;
        case D_RIGHT:
            Player_move(player, PLAYER_STEP_SIZE, 0);
            break;
        default:
            break;
    }
}