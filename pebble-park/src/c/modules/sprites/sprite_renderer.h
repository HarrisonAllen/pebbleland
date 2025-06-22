#pragma once

#include <pebble.h>
#include "../../pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"
#include "player_sprites.h"

static bool s_debug = true;

static void draw_rect(GContext *ctx, GRect rect, GColor color) {
    graphics_context_set_stroke_color(ctx, color);
    graphics_context_set_fill_color(ctx, color);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);
}

static void render_tile(GContext *ctx, uint8_t *tile, uint8_t *palette, uint8_t pixel_size, GPoint start) {
    int x, y;
    uint8_t pixel_byte, pixel;
    uint16_t offset;
    uint8_t shift;
    GColor color;
    GRect pixel_bounds;
    for (y = 0; y < GBC_TILE_HEIGHT; y++) {
        for (x = 0; x < GBC_TILE_WIDTH; x++) {
            offset = (y << 2) + (x >> 1); // pixel y * 4 + pixel_x / 2
            pixel_byte = tile[offset];
            shift = (1 ^ (x & 1)) << 2; // (1 - pixel_x % 2) * 4
            pixel = 0b1111 & (pixel_byte >> shift);
            if (pixel != 0) {
                color = (GColor){.argb=palette[pixel]};
                pixel_bounds = GRect(start.x + x*pixel_size, start.y + y*pixel_size, pixel_size, pixel_size);
                draw_rect(ctx, pixel_bounds, color);
            }
        }
    }
}

static void render_sprite(GContext *ctx, GBC_Graphics *graphics, uint8_t tile_start, uint8_t sprite_width, uint8_t sprite_height, uint8_t *palette, uint8_t vram, GPoint start, uint8_t pixel_size) {
    int x_tile, y_tile;
    int x_pos = start.x;
    int y_pos = start.y;
    int tile_offset;
    uint8_t *tile;
    for (y_tile = 0; y_tile < sprite_height; y_tile++) {
        y_pos = start.y + y_tile * pixel_size * GBC_TILE_HEIGHT;
        for (x_tile = 0; x_tile < sprite_width; x_tile++) {
            x_pos = start.x + x_tile * pixel_size * GBC_TILE_WIDTH;
            tile_offset = y_tile * sprite_height + x_tile;
            tile = GBC_Graphics_get_tile(graphics, vram, tile_start + tile_offset);
            render_tile(ctx, tile, palette, pixel_size, GPoint(x_pos, y_pos));
        }
    }
}

static void render_outfit(GContext *ctx, GBC_Graphics *graphics, uint8_t hair_tile, uint8_t clothes_tile, uint8_t *palette, GPoint start, uint8_t pixel_size) {
    render_sprite(ctx, graphics, hair_tile, HAIR_WIDTH_TILES, HAIR_HEIGHT_TILES, palette, PLAYER_VRAM, start, pixel_size);
    render_sprite(ctx, graphics, clothes_tile, CLOTHES_WIDTH_TILES, CLOTHES_HEIGHT_TILES, palette, PLAYER_VRAM, GPoint(start.x, start.y + HAIR_HEIGHT_PIXELS * pixel_size), pixel_size);
}