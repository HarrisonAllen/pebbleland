#pragma once
#include <pebble.h>
#include "sprites/player_sprites.h"

static bool do_rects_overlap(GRect rect1, GRect rect2) {
    int left_x = rect1.origin.x;
    int right_x = left_x + rect1.size.w;
    int top_y = rect1.origin.y;
    int bottom_y = top_y + rect1.size.h;
    GPoint top_left = GPoint(left_x, top_y);
    GPoint top_right = GPoint(right_x, top_y);
    GPoint bottom_left = GPoint(left_x, bottom_y);
    GPoint bottom_right = GPoint(right_x, bottom_y);
    return grect_contains_point(&rect2, &top_left)
           || grect_contains_point(&rect2, &top_right)
           || grect_contains_point(&rect2, &bottom_left)
           || grect_contains_point(&rect2, &bottom_right);
}

static int clamp(int low, int value, int high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

static int make_clothes_sprite_offset(int shirt, int pants) {
    return (shirt + pants * CLOTHES_NUM_SHIRTS) * CLOTHES_TILES_PER_SPRITE;
}