#pragma once

#include <pebble.h>

// For future me reference:
// * The color palette was generated from the entire (color) world map
// * The b/w palette was derived from a modified and grayscaled version of the color tilesheet
uint8_t BG_PALETTE[16] = {
    #if defined(PBL_COLOR)
        0b11000000, 0b11000000, 0b11000100, 0b11000101, 0b11000110, 0b11001001, 0b11001011, 0b11010000, 0b11010101, 0b11100100, 0b11100101, 0b11101010, 0b11110000, 0b11111001, 0b11111110, 0b11111111,
    #else
        0b00, 0b00, 0b10, 0b11, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00, 0b00,
    #endif
};

uint8_t BLANK_BG_PALETTE[16] = {
    #if defined(PBL_COLOR)
        0b11101111, 0b11101111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
    #else
        0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11, 0b11,
    #endif
};