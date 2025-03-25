#pragma once
#include <pebble.h>
#include "../defines.h"
#include "../player.h"
#include "user_menu_item.h"

typedef struct _user_menu UserMenu;
struct _user_menu {
    Window *window;
    // void *callback;
    void (*callback)(int );
    SimpleMenuLayer *menu_layer;
    SimpleMenuItem menu_items[MAX_PLAYERS];
    SimpleMenuItem offline_placeholder_item[1];
    SimpleMenuSection menu_sections[2];
    Player **players;
    UserMenuItem *user_menu_items[MAX_PLAYERS];
};

UserMenu *UserMenu_init(Player **players);

void UserMenu_destroy(UserMenu *user_menu);

void UserMenu_open_menu(UserMenu *user_menu);