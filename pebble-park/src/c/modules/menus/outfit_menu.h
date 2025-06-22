#pragma once

#include <pebble.h>
#include "../player.h"
#include "../sprites/player_sprites.h"
#include "../defines.h"

typedef struct _outfit_menu OutfitMenu;
struct _outfit_menu {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuItem menu_items[OUTFIT_MENU_NUM_ITEMS];
    SimpleMenuSection menu_sections[1];
    Player *player;
    Layer *sprite_layer;
    TextLayer *text_layer;
    int hair, shirt, pants;
    int colors[4];
};

OutfitMenu *OutfitMenu_init(Player *player);

void OutfitMenu_destroy(OutfitMenu *outfit_menu);

void OutfitMenu_open_menu(OutfitMenu *outfit_menu);