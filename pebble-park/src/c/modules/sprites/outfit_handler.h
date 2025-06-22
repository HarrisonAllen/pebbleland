#pragma once
#include <pebble.h>
#include "../../pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"
#include "player_sprites.h"

static void load_outfit(int hair_number, int clothes_number, bool all_sprites, int vram_offset, int sprite_number, int vram_number, GBC_Graphics *graphics) {
    int hair_position = hair_number * HAIR_TILES_PER_HAIR;
    int clothes_position = clothes_number * CLOTHES_TILES_PER_CLOTHES;
    // if player one, load head D, clothes D, head L, clothes L, head U, clothes U
    // otherwise load head D, clothes D
    int vram_position = vram_offset;
    int sprites_to_load = all_sprites ? OUTFITS_PER : 1;
    for (int i = 0; i < sprites_to_load; i++) {
        // Load head, one per direction
        int hair = 0;
        if (i < CLOTHES_LEFT_SPRITE_START) {
            hair = 0;
        } else if (i < CLOTHES_UP_SPRITE_START) {
            hair = 1;
        } else {
            hair = 2;
        }
        int hair_offset = (hair * HAIR_TILES_PER_SPRITE);
        GBC_Graphics_load_from_tilesheet_into_vram(graphics, HAIR_TILESHEET, hair_position + hair_offset, HAIR_TILES_PER_SPRITE, vram_position, vram_number);
        vram_position += HAIR_TILES_PER_SPRITE;

        // Load clothes, two per direction
        int clothes_offset = CLOTHES_TILES_PER_SPRITE * i;
        GBC_Graphics_load_from_tilesheet_into_vram(graphics, CLOTHES_TILESHEET, clothes_position + clothes_offset, CLOTHES_TILES_PER_SPRITE, vram_position, vram_number);
        vram_position += CLOTHES_TILES_PER_SPRITE;
    }
}