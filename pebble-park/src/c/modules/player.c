#include "player.h"
#include "communication.h"

Player *Player_initialize(int number, GBC_Graphics *graphics, ClaySettings *settings) {
    Player *player = NULL;
    player = malloc(sizeof(Player));
    if (player == NULL)
        return NULL;
    player->number = number;
    player->graphics = graphics;
    player->settings = settings;
    player->active = false;
    player->sprite_number = MAX_PLAYERS - (player->number + 1);
    player->direction = D_DOWN;
    if (player->number == 0) {
        player->tile_offset = 0;
        player->num_tiles = PLAYER_ONE_NUM_TILES;
    } else {
        player->tile_offset = PLAYER_ONE_NUM_TILES + (player->number - 1) * PLAYER_SPRITE_NUM_TILES;
        player->num_tiles = PLAYER_SPRITE_NUM_TILES;
    }
    GBC_Graphics_oam_set_sprite(player->graphics, player->sprite_number, 0, 0, player->tile_offset, GBC_Graphics_attr_make(0, PLAYER_VRAM, false, false, true), PLAYER_SPRITE_TILE_WIDTH - 1, PLAYER_SPRITE_TILE_HEIGHT - 1, 0, 0);
    Player_render(player);
    return player;
}

void Player_destroy(Player *player) {
    if (player != NULL) {
        free(player);
    }
}

void Player_push_input(Player *player, QueuedInput input) {
    if (input == Q_SELECT_PRESS) {
        player->select_pressed = true;
    } else if (input == Q_SELECT_RELEASE) {
        player->select_pressed = false;
    } else {
        player->queued_input = input;
    }
}

void Player_pop_input(Player *player) {
    if (player->settings->Tilt) {
        Player_set_direction(player, player->tilt_direction);
    }
    if (player->queued_input == Q_UP) {
        Player_rotate_counterclockwise(player);
        player->queued_input = Q_NONE;
    } 
    if (player->queued_input == Q_DOWN) {
        Player_rotate_clockwise(player);
        player->queued_input = Q_NONE;
    }
    if (player->queued_input == Q_SELECT) {
        if (player->state == P_WALK) {
            if (player->walk_state == W_WALK) {
                // Autowalking? Select to stop
                player->walk_state = W_STAND;
                player->state = P_STAND;
            } else if (player->walk_state == W_STEP) {
                // Mid step? Double click -> autowalk
                player->walk_state = W_WALK;
            }
        } else if (player->state == P_STAND) {
            // Check if player in front?
            // TODO
            // else:
            // Standing? Take a step
            player->state = P_WALK;
            player->walk_state = W_STEP;
            player->walk_frame = 0;
        }
    } else if (player->queued_input == Q_NONE) {
        if (player->state == P_WALK) {
            if (player->walk_state == W_STEP) {
                // Nothing pressed, in step, so stop    
                player->state = P_STAND;
                player->walk_state = W_STAND;
            }
        }
    }
    player->queued_input = Q_NONE;
}

void Player_step(Player *player) {
    if (player->state == P_STAND) {
        Player_pop_input(player);
    } else if (player->state == P_WALK) {
        Player_take_step(player);
        player->walk_frame = (player->walk_frame + 1) % PLAYER_WALK_FRAME_COUNT;

        if (player->walk_frame == 0) {
            Player_pop_input(player);
            broadcast_position(player->x, player->y);
        }
    }
    Player_render(player);
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
    GBC_Graphics_load_from_buffer_into_vram(player->graphics, sprite_data, player->num_tiles, player->tile_offset, PLAYER_VRAM);
    GBC_Graphics_set_sprite_palette_array(player->graphics, player->sprite_number, palette_data);
}

void Player_render(Player *player) {
    if (player->number == 0) {
        GBC_Graphics_oam_set_sprite_tile(player->graphics, player->sprite_number, player->tile_offset + PLAYER_SPRITE_NUM_TILES * (player->direction * 2 + player->walk_frame % 2));
    } else {
        GBC_Graphics_oam_set_sprite_tile(player->graphics, player->sprite_number, player->tile_offset);
    }
}

void Player_set_direction(Player *player, Direction direction) {
    if (direction != D_MAX) {
        player->direction = direction;
        Player_render(player);
    }
}

void Player_rotate_clockwise(Player *player) {
    Player_set_direction(player, (player->direction + 1 + D_MAX) % D_MAX);
}

void Player_rotate_counterclockwise(Player *player) {
    Player_set_direction(player, (player->direction - 1 + D_MAX) % D_MAX); // The + D_MAX is to avoid negative mod
}

void Player_set_tilt_direction(Player *player, Direction tilt_direction) {
    player->tilt_direction = tilt_direction;
}

void Player_take_step(Player *player) {
    switch (player->direction) {
        case D_UP:
            Player_move(player, 0, -PLAYER_SUB_STEP_SIZE);
            break;
        case D_DOWN:
            Player_move(player, 0, PLAYER_SUB_STEP_SIZE);
            break;
        case D_LEFT:
            Player_move(player, -PLAYER_SUB_STEP_SIZE, 0);
            break;
        case D_RIGHT:
            Player_move(player, PLAYER_SUB_STEP_SIZE, 0);
            break;
        default:
            break;
    }
    Player_render(player);
}