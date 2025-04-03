#include "player.h"
#include "communication.h"
#include "utility.h"

Player *Player_initialize(int number, GBC_Graphics *graphics, Background *background, ClaySettings *settings, Player *player_one, Window *window) {
    Player *player = NULL;
    player = malloc(sizeof(Player));
    if (player == NULL)
        return NULL;
    player->number = number;
    player->graphics = graphics;
    player->background = background;
    player->settings = settings;
    player->window = window;
    player->active = false;
    player->sprite_number = MAX_PLAYERS - (player->number + 1);
    player->direction = D_DOWN;
    if (player->number == 0) {
        player->tile_offset = 0;
        player->num_tiles = PLAYER_ONE_NUM_TILES;
        player->player_one = player;
    } else {
        player->tile_offset = PLAYER_ONE_NUM_TILES + (player->number - 1) * PLAYER_SPRITE_NUM_TILES;
        player->num_tiles = PLAYER_SPRITE_NUM_TILES;
        player->player_one = player_one;
    }
    GBC_Graphics_oam_set_sprite(player->graphics, player->sprite_number, 0, 0, player->tile_offset, GBC_Graphics_attr_make(0, PLAYER_VRAM, false, false, true), PLAYER_SPRITE_TILE_WIDTH - 1, PLAYER_SPRITE_TILE_HEIGHT - 1, 0, 0);
    Player_render(player);
    return player;
}

void Player_destroy(Player *player) {
    if (player->text_layer != NULL) {
        text_layer_destroy(player->text_layer);
    }
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
                if (player->number == 0) {
                    Background_load_tiles_in_direction(player->background, player->direction, player->x, player->y);
                }
            }
        } else if (player->state == P_STAND) {
            // Check if player in front?
            // TODO
            // else:
            // Standing? Take a step
            player->state = P_WALK;
            player->walk_state = W_STEP;
            player->walk_frame = 0;
            if (player->number == 0) {
                Background_load_tiles_in_direction(player->background, player->direction, player->x, player->y);
            }
        }
    } else if (player->queued_input == Q_NONE) {
        if (player->state == P_WALK) {
            if (player->walk_state == W_STEP) {
                // Nothing pressed, in step, so stop    
                player->state = P_STAND;
                player->walk_state = W_STAND;
            } else {
                if (player->number == 0) {
                    Background_load_tiles_in_direction(player->background, player->direction, player->x, player->y);
                }
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

void Player_set_up_username_text(Player *player) {
    if (player->text_layer != NULL) {
        text_layer_destroy(player->text_layer);
    }
    
    Layer *window_layer = window_get_root_layer(player->window);
    GRect bounds = layer_get_bounds(window_layer);
    GFont font = fonts_get_system_font(PLAYER_FONT);
    
    // Find the bounds of the scrolling text
    GRect text_max_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h); // TODO: could optimize this
    GSize text_size = graphics_text_layout_get_content_size(player->username, font, 
                    text_max_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter);
    GRect text_bounds = GRect(0, 0, text_size.w, text_size.h + 4); // TODO: get rid of this magic number to prevent text cutoff (aka text box too small)
    
    // Create the TextLayer
    player->text_layer = text_layer_create(text_bounds);
    text_layer_set_overflow_mode(player->text_layer, GTextOverflowModeWordWrap);
    text_layer_set_font(player->text_layer, font);
    text_layer_set_text(player->text_layer, player->username);
    text_layer_set_background_color(player->text_layer, GColorWhite);
    layer_add_child(window_layer, text_layer_get_layer(player->text_layer));
    // TODO: I think the text layer should be a child of a game layer, 
    // otheriwse it will go on top of other layers as players pop in?
    Player_render_username(player);
}

void Player_activate(Player *player) {
    player->active = true;
    Player_set_up_username_text(player);
    Player_show(player);
}

void Player_set_username(Player *player, char *username) {
    strcpy(player->username, username);
}

void Player_set_position(Player *player, int x, int y) {
    x = clamp(PLAYER_MIN_X, x, PLAYER_MAX_X);
    y = clamp(PLAYER_MIN_Y, y, PLAYER_MAX_Y);
    if (player->number == 0) {
        GBC_Graphics_oam_set_sprite_pos(player->graphics, 
                                    player->sprite_number, 
                                    PLAYER_SPRITE_SCREEN_X_OFFSET,
                                    PLAYER_SPRITE_SCREEN_Y_OFFSET);
        Background_move(player->background, x - player->x, y - player->y);
    }
    player->x = x;
    player->y = y;
}

void Player_move(Player *player, int x, int y) {
    Player_set_position(player, player->x + x, player->y + y);
}

GPoint Player_get_screen_position(Player *player) {
    int x = GBC_Graphics_get_screen_x_origin(player->graphics)
            + GBC_Graphics_oam_get_sprite_x(player->graphics, player->sprite_number)
            - GBC_SPRITE_OFFSET_X
            + PLAYER_SPRITE_WIDTH / 2;
    int y = GBC_Graphics_get_screen_y_origin(player->graphics)
            + GBC_Graphics_oam_get_sprite_y(player->graphics, player->sprite_number)
            - GBC_SPRITE_OFFSET_Y
            + PLAYER_SPRITE_HEIGHT / 2;
    return GPoint(x, y);
}

void Player_load_sprite_and_palette(Player *player, uint8_t *sprite_data, uint8_t *palette_data) {
    GBC_Graphics_load_from_buffer_into_vram(player->graphics, sprite_data, player->num_tiles, player->tile_offset, PLAYER_VRAM);
    GBC_Graphics_set_sprite_palette_array(player->graphics, player->sprite_number, palette_data);
}

static bool is_player_on_screen(int player_x, int player_y, int player_one_x, int player_one_y) {
    int player_x_offset = player_x - player_one_x;
    int player_y_offset = player_y - player_one_y;
    return player_x_offset >= PLAYER_SCREEN_MIN_OFFSET_X
           && player_x_offset <= PLAYER_SCREEN_MAX_OFFSET_X
           && player_y_offset >= PLAYER_SCREEN_MIN_OFFSET_Y
           && player_y_offset <= PLAYER_SCREEN_MAX_OFFSET_Y;
}

void Player_render_username(Player *player) {
    if (player->text_layer == NULL) return;
    Layer *layer = text_layer_get_layer(player->text_layer);
    if (player->number == 0 || !is_player_on_screen(player->x, player->y, player->player_one->x, player->player_one->y)) {
        layer_set_hidden(layer, true);
    } else {
        GRect text_bounds = layer_get_bounds(layer);
        // GSize text_size = GSize(bounds.size.w, bounds.size.h / 2);
        GPoint player_center = Player_get_screen_position(player);
        GPoint text_center = GPoint(player_center.x, player_center.y - PLAYER_SPRITE_HEIGHT / 2 - text_bounds.size.h / 2);
        GPoint text_top_left = GPoint(text_center.x - text_bounds.size.w / 2, text_center.y - text_bounds.size.h / 2);
        GRect new_frame = GRect(text_top_left.x, text_top_left.y, text_bounds.size.w, text_bounds.size.h);
        layer_set_frame(layer, new_frame);
        layer_set_hidden(layer, false);
        // TODO: hide username if off screen
        //  - This should also fix the opposite side of the world but can see other players nameplate issue
    }
}

void Player_render(Player *player) {
    if (!player->active) return;
    if (player->number == 0) {
        GBC_Graphics_oam_set_sprite_tile(player->graphics, player->sprite_number, player->tile_offset + PLAYER_SPRITE_NUM_TILES * (player->direction * 2 + player->walk_frame % 2));
    } else {
        GBC_Graphics_oam_set_sprite_tile(player->graphics, player->sprite_number, player->tile_offset);
        int player_x_offset = player->x - player->player_one->x;
        int player_y_offset = player->y - player->player_one->y;
        GBC_Graphics_oam_set_sprite_hidden(player->graphics, player->sprite_number, 
            !is_player_on_screen(player->x, player->y, player->player_one->x, player->player_one->y));
        GBC_Graphics_oam_set_sprite_pos(player->graphics, 
            player->sprite_number, 
            PLAYER_SPRITE_SCREEN_X_OFFSET + player_x_offset,
            PLAYER_SPRITE_SCREEN_Y_OFFSET + player_y_offset);
        Player_render_username(player);
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