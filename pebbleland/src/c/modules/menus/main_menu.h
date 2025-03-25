#pragma once

#include <pebble.h>

typedef struct _main_menu MainMenu;
struct _main_menu {
    Window *window;
    // void *callback;
    void (*callback)(int );
    SimpleMenuLayer *menu_layer;
    SimpleMenuItem menu_items[4];
    SimpleMenuSection menu_sections[1];
};

MainMenu *MainMenu_init();

void MainMenu_destroy(MainMenu *main_menu);

void MainMenu_open_menu(MainMenu *main_menu);