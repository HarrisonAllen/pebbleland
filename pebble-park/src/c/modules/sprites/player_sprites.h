#pragma once
#include <pebble.h>

// Files
#define HAIR_TILESHEET RESOURCE_ID_PLAYER_HAIRS
#define CLOTHES_TILESHEET RESOURCE_ID_PLAYER_CLOTHES

// Hairs
#define HAIR_COUNT 10
#define HAIR_SPRITES_PER 3 // one for each direction, right will be left flipped
#define HAIR_NUM_SPRITES (HAIR_COUNT * HAIR_SPRITES_PER)
#define HAIR_WIDTH_TILES 2 // in tiles
#define HAIR_HEIGHT_TILES 2 // in tiles
#define HAIR_WIDTH_PIXELS (HAIR_WIDTH_TILES * GBC_TILE_WIDTH)
#define HAIR_HEIGHT_PIXELS (HAIR_HEIGHT_TILES * GBC_TILE_HEIGHT)
#define HAIR_TILES_PER_SPRITE (HAIR_WIDTH_TILES * HAIR_HEIGHT_TILES)
#define HAIR_TILES_PER_HAIR (HAIR_TILES_PER_SPRITE * HAIR_SPRITES_PER)
#define HAIR_TOTAL_TILES (HAIR_COUNT * HAIR_TILES_PER_HAIR)
#define HAIR_VRAM_START 0

// Clothes
#define CLOTHES_COUNT 10
#define CLOTHES_NUM_SHIRTS 5
#define CLOTHES_NUM_PANTS 2
#define CLOTHES_SPRITES_PER 6 // 4 for up down, 2 for left right
#define CLOTHES_NUM_SPRITES (CLOTHES_COUNT * CLOTHES_SPRITES_PER)
#define CLOTHES_WIDTH_TILES 2
#define CLOTHES_HEIGHT_TILES 1
#define CLOTHES_WIDTH_PIXELS (CLOTHES_WIDTH_TILES * GBC_TILE_WIDTH)
#define CLOTHES_HEIGHT_PIXELS (CLOTHES_HEIGHT_TILES * GBC_TILE_HEIGHT)
#define CLOTHES_TILES_PER_SPRITE (CLOTHES_WIDTH_TILES * CLOTHES_HEIGHT_TILES)
#define CLOTHES_TILES_PER_CLOTHES (CLOTHES_TILES_PER_SPRITE * CLOTHES_SPRITES_PER)
#define CLOTHES_TILES_PER_STEP (CLOTHES_COUNT * CLOTHES_TILES_PER_SPRITE)
#define CLOTHES_TILES_PER_DIRECTION (CLOTHES_TILES_PER_STEP * 2)
#define CLOTHES_TOTAL_TILES (CLOTHES_COUNT * CLOTHES_TILES_PER_CLOTHES)
#define CLOTHES_DOWN_SPRITE_START 0
#define CLOTHES_LEFT_SPRITE_START 3
#define CLOTHES_UP_SPRITE_START 5
#define CLOTHES_VRAM_START (HAIR_VRAM_START + HAIR_TOTAL_TILES)

// Outfits (hair + clothes)
#define OUTFIT_WIDTH 2
#define OUTFIT_HEIGHT 3
#define OUTFIT_WIDTH_PIXELS (OUTFIT_WIDTH * GBC_TILE_WIDTH)
#define OUTFIT_HEIGHT_PIXELS (OUTFIT_HEIGHT * GBC_TILE_HEIGHT)
#define OUTFIT_TILES (OUTFIT_WIDTH * OUTFIT_HEIGHT)
#define OUTFITS_PER CLOTHES_SPRITES_PER
#define OUTFITS_TOTAL_TILES (OUTFITS_PER * OUTFIT_TILES)

// Features
#define OUTFIT_NUM_FEATURES 11
#define OUTFIT_NUM_SHOES 1
#define OUTFIT_NUM_LEGS 2
#define OUTFIT_NUM_SHIRTS 5

// Outfit menu
#define OUTFIT_MENU_NUM_ITEMS 5
#define MENU_ITEM_HAIR 0
#define MENU_ITEM_SHIRT 1
#define MENU_ITEM_PANTS 2
#define MENU_ITEM_SHOES 3
#define MENU_ITEM_SAVE_OUTFIT 4
#define PREVIEW_OUTFIT_PIXEL_SIZE 4
#define PREVIEW_OUTFIT_WIDTH (PREVIEW_OUTFIT_PIXEL_SIZE * OUTFIT_WIDTH_PIXELS)
#define PREVIEW_OUTFIT_HEIGHT (PREVIEW_OUTFIT_PIXEL_SIZE * OUTFIT_HEIGHT_PIXELS)
#define OUTFIT_TEXT_LAYER_HEIGHT 24
#define OUTFIT_TEXT_LAYER_FONT FONT_KEY_GOTHIC_18
#define OUTFIT_SUB_MENU_NUM_ITEMS 2
#define SUB_MENU_ITEM_STYLE 0
#define SUB_MENU_ITEM_COLOR 1

// Palettes
#define PLAYER_PALETTE_OUTLINE 1
#define PLAYER_PALETTE_BODY 2
#define PLAYER_PALETTE_SHOE_MAJOR 3
#define PLAYER_PALETTE_SHOE_MINOR 4
#define PLAYER_PALETTE_LEGS_MAJOR 5
#define PLAYER_PALETTE_LEGS_MINOR 6
#define PLAYER_PALETTE_SHIRT_MAJOR 7
#define PLAYER_PALETTE_SHIRT_MINOR 8
#define PLAYER_PALETTE_SHIRT_ACCENT 9
#define PLAYER_PALETTE_HAIR_MAJOR 10
#define PLAYER_PALETTE_HAIR_MINOR 11

// Colors

#if defined(PBL_COLOR)
    #define COLORS_NULL 0
    #define COLORS_BASE 0b11000000
    #define COLORS_RED_MAJOR 0b11100000
    #define COLORS_RED_MINOR 0b11110000
    #define COLORS_ORANGE_MAJOR 0b11010000
    #define COLORS_ORANGE_MINOR 0b11110100
    #define COLORS_YELLOW_MAJOR 0b11111000
    #define COLORS_YELLOW_MINOR 0b11111100
    #define COLORS_GREEN_MAJOR 0b11000100
    #define COLORS_GREEN_MINOR 0b11011001
    #define COLORS_BLUE_MAJOR 0b11000011
    #define COLORS_BLUE_MINOR 0b11001011
    #define COLORS_PURPLE_MAJOR 0b11010010
    #define COLORS_PURPLE_MINOR 0b11100111
    #define COLORS_PINK_MAJOR 0b11110011
    #define COLORS_PINK_MINOR 0b11111011
    #define COLORS_GRAY_MAJOR 0b11010101
    #define COLORS_GRAY_MINOR 0b11101010
    #define COLORS_BLACK_MAJOR 0b11000000
    #define COLORS_BLACK_MINOR 0b11010101
    #define COLORS_ACCENT 0b11111111
    #define COLORS_BW_BASE_MAJOR 0b11000000
    #define COLORS_BW_BASE_MINOR 0b11111111
    #define COLORS_BW_ACCENT 0b11000000
    #define COLORS_SKIN_MAJOR 0b11000000
    #define COLORS_SKIN_MINOR 0b11111111
    #define NUM_COLORS 9
#else
    #define COLORS_NULL 0xF
    #define COLORS_BASE GBC_COLOR_BLACK
    #define COLORS_SKIN_MAJOR GBC_COLOR_BLACK
    #define COLORS_SKIN_MINOR GBC_COLOR_WHITE
    #define NUM_COLORS 1
#endif // defined(PBL_COLOR)

// // Names
// const char *FEATURE_NAMES[11] = {
//     "Outline",
//     "Body",
//     "Shoes Primary",
//     "Shoes Secondary",
//     "Legs Primary",
//     "Legs Secondary",
//     "Shirt Primary",
//     "Shirt Secondary",
//     "Shirt Accent",
//     "Hair Primary",
//     "Hair Secondary"
// };