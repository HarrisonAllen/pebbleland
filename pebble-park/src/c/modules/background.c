#include "background.h"
#include "defines.h"
#include "palettes/bg_palettes.h"
#include "utility.h"

uint8_t get_tile_id_from_map(uint8_t *map, int tile_x, int tile_y) {
    tile_x = clamp(0, tile_x, BG_TILE_WIDTH - 1);
    tile_y = clamp(0, tile_y, BG_TILE_HEIGHT - 1);
    return map[tile_x + tile_y * BG_TILE_WIDTH];
}
  
void Background_load_tiles(Background *background, uint8_t bg_root_x, uint8_t bg_root_y, int tile_root_x, int tile_root_y, uint8_t num_x_tiles, uint8_t num_y_tiles) {
    for (int tile_y = tile_root_y; tile_y < tile_root_y + num_y_tiles; tile_y++) {
        for (int tile_x = tile_root_x; tile_x < tile_root_x + num_x_tiles; tile_x++) {
            uint8_t tile = get_tile_id_from_map(background->tilemap, tile_x, tile_y);
            GBC_Graphics_bg_set_tile_and_attrs(background->graphics, 0, bg_root_x + (tile_x - tile_root_x), bg_root_y + (tile_y - tile_root_y), tile, 0);
        }
    }
}

void Background_load_screen(Background *background, int player_x, int player_y) {
    GBC_Graphics_bg_set_scroll_pos(background->graphics, 0, 0, 0);
    uint8_t bg_root_x = GBC_Graphics_bg_get_scroll_x(background->graphics, 0) >> 3;
    uint8_t bg_root_y = GBC_Graphics_bg_get_scroll_y(background->graphics, 0) >> 3;
    int tile_root_x = ((player_x + BG_PLAYER_OFFSET_X) / GBC_TILE_WIDTH);
    int tile_root_y = ((player_y + BG_PLAYER_OFFSET_Y) / GBC_TILE_HEIGHT);
    Background_load_tiles(background, bg_root_x & 31, bg_root_y & 31, tile_root_x, tile_root_y, SCREEN_TILE_WIDTH, SCREEN_TILE_HEIGHT);
}

void Background_load_tiles_in_direction(Background *background, Direction direction, int player_x, int player_y) {
    uint8_t bg_root_x = (GBC_Graphics_bg_get_scroll_x(background->graphics, 0) >> 3);
    uint8_t bg_root_y = (GBC_Graphics_bg_get_scroll_y(background->graphics, 0) >> 3);
    int tile_root_x = ((player_x + BG_PLAYER_OFFSET_X) / GBC_TILE_WIDTH);
    int tile_root_y = ((player_y + BG_PLAYER_OFFSET_Y) / GBC_TILE_HEIGHT);
    uint16_t num_x_tiles = 0, num_y_tiles = 0;
    switch(direction) {
        case D_UP:
            tile_root_y -= 2;
            bg_root_y -= 2;
            num_x_tiles = SCREEN_TILE_WIDTH;
            num_y_tiles = 2;
            break;
        case D_LEFT:
            tile_root_x -= 2;
            bg_root_x -= 2;
            num_x_tiles = 2;
            num_y_tiles = SCREEN_TILE_HEIGHT;
            break;
        case D_DOWN:
            tile_root_y += SCREEN_TILE_HEIGHT;
            bg_root_y += SCREEN_TILE_HEIGHT;
            num_x_tiles = SCREEN_TILE_WIDTH;
            num_y_tiles = 2;
            break;
        case D_RIGHT:
            tile_root_x += SCREEN_TILE_WIDTH;
            bg_root_x += SCREEN_TILE_WIDTH;
            num_x_tiles = 2;
            num_y_tiles = SCREEN_TILE_HEIGHT;
            break;
        default:
            break;
    }
    Background_load_tiles(background, bg_root_x & 31, bg_root_y & 31, tile_root_x, tile_root_y, num_x_tiles, num_y_tiles);
  }


void Background_load_resources(Background *background) {
    GBC_Graphics_load_entire_tilesheet_into_vram(background->graphics, RESOURCE_ID_BG_TILESHEET, 0, BG_VRAM);
    GBC_Graphics_set_bg_palette_array(background->graphics, 0, BG_PALETTE);

    ResHandle handle = resource_get_handle(RESOURCE_ID_BG_TILEMAP);
    size_t res_size = resource_size(handle);
    if (background->tilemap != NULL) {
        free(background->tilemap);
        background->tilemap = NULL;
    }
    background->tilemap = (uint8_t*)malloc(res_size);
    resource_load(handle, background->tilemap, res_size);
}

Background *Background_init(GBC_Graphics *graphics) {
    Background *background = NULL;
    background = malloc(sizeof(Background));
    if (background == NULL)
        return NULL;
    background->graphics = graphics;

    return background;
}

void Background_destroy(Background *background) {
    if (background != NULL) {
        if (background->tilemap != NULL) {
            free(background->tilemap);
            background->tilemap = NULL;
        }
        free(background);
    }
}

void Background_move(Background *background, int x, int y) {
    GBC_Graphics_bg_move(background->graphics, 0, x, y);
}