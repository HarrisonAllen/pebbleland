// Eventually this will get deprecated in favor of custom sprites
#pragma once

#include <pebble.h>

uint8_t DEFAULT_PALETTE[16] = {
    #if defined(PBL_COLOR)
        0b11000000, 0b11000000, 0b11111111, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000,
    #else
        0b00, 0b00, 0b11, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00,
    #endif
};

uint8_t PLAYER_ONE_PALETTE[16] = {
    #if defined(PBL_COLOR)
        0b11000000, 0b11000000, 0b11110000, 0b11111111, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000,
    #else
        0b00, 0b00, 0b00, 0b11, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00,
    #endif
};

uint8_t BLANK_BG_PALETTE[16] = {
    #if defined(PBL_COLOR)
        0b11101111, 0b11101111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
    #else
        0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11,
    #endif
};


// #define DEFAULT_DEFAULT 0

// const uint8_t DEFAULT_DATA[] = {
//     // vram tile start, width, height
//     0, 1, 2, // default
// };

// static uint16_t load_default(GBC_Graphics *gbc_graphics, uint8_t vram_bank, uint8_t vram_start_offset, uint8_t palette_num, uint8_t *num_sprites, const uint8_t **sprite_data) {
//     // Load default tilesheet
//     uint16_t num_loaded_tiles = load_tilesheet(gbc_graphics, RESOURCE_ID_DATA_DEFAULT_TILESHEET, 0, vram_start_offset, vram_bank);

//     // Load default palette
//     GBC_Graphics_set_sprite_palette_array(gbc_graphics, palette_num, DEFAULT_PALETTES[0]);

//     // Set number of sprites
//     *num_sprites = 1;

//     // Set sprite data
//     *sprite_data = DEFAULT_DATA;
    
//     return num_loaded_tiles;
// }

// #define PLAYER_ONE_1_UP 0
// #define PLAYER_ONE_2_RIGHT 1
// #define PLAYER_ONE_3_DOWN 2
// #define PLAYER_ONE_4_LEFT 3

// const uint8_t PLAYER_ONE_DATA[] = {
//     // vram tile start, width, height
//     0, 1, 2, // 1_up
//     6, 1, 2, // 2_right
//     12, 1, 2, // 3_down
//     18, 1, 2, // 4_left
// };

// static uint16_t load_player_one(GBC_Graphics *gbc_graphics, uint8_t vram_bank, uint8_t vram_start_offset, uint8_t palette_num, uint8_t *num_sprites, const uint8_t **sprite_data) {
//     // Load player_one tilesheet
//     uint16_t num_loaded_tiles = load_tilesheet(gbc_graphics, RESOURCE_ID_DATA_PLAYER_ONE_TILESHEET, 0, vram_start_offset, vram_bank);

//     // Load player_one palette
//     GBC_Graphics_set_sprite_palette_array(gbc_graphics, palette_num, PLAYER_ONE_PALETTES[0]);

//     // Set number of sprites
//     *num_sprites = 4;

//     // Set sprite data
//     *sprite_data = PLAYER_ONE_DATA;
    
//     return num_loaded_tiles;
// }

// #define NUM_SPRITE_GROUPS 2

// uint16_t (*LOAD_SPRITE_GROUP[2])(GBC_Graphics *gbc_graphics, uint8_t vram_bank, uint8_t vram_start_offset, uint8_t palette_num, uint8_t *num_sprites, const uint8_t **sprite_data) = {
//     load_default,
//     load_player_one,
// };

    