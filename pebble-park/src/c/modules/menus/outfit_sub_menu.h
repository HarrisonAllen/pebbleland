#pragma once

#include <pebble.h>
#include "../player.h"
#include "../sprites/player_sprites.h"
#include "../defines.h"

typedef struct _outfit_sub_menu OutfitSubMenu;
struct _outfit_sub_menu {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuItem menu_items[OUTFIT_SUB_MENU_NUM_ITEMS];
    SimpleMenuSection menu_sections[1];
    void (*update_callback)(int, void*);
    void (*close_callback)(void*);
    void *context;
    Layer *sprite_layer;
    TextLayer *text_layer;
};

OutfitSubMenu *OutfitSubMenu_init(void (*update_callback)(int, void*), void (*close_callback)(void*), void *context, Layer *sprite_layer, TextLayer *text_layer);

void OutfitSubMenu_destroy(OutfitSubMenu *outfit_sub_menu);

void OutfitSubMenu_open_menu(OutfitSubMenu *outfit_sub_menu);