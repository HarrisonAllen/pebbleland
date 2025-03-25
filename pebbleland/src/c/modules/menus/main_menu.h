#pragma once

#include <pebble.h>
#include "../player.h"
#include "user_menu_item.h"
#include "../defines.h"

typedef struct _main_menu MainMenu;
struct _main_menu {
    Window *window;
    // void *callback;
    void (*callback)(int );
    SimpleMenuLayer *menu_layer;
    SimpleMenuItem menu_items[MAIN_MENU_NUM_ITEMS];
    SimpleMenuSection menu_sections[1];
    Player **players;
    UserMenuItem *user_menu_item;
};

MainMenu *MainMenu_init(Player **players);

void MainMenu_destroy(MainMenu *main_menu);

void MainMenu_open_menu(MainMenu *main_menu);