#pragma once

#include <pebble.h>
#include "../player.h"
#include "../settings.h"
#include "../defines.h"

typedef struct _settings_menu SettingsMenu;
struct _settings_menu {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuItem menu_items[SETTINGS_MENU_NUM_ITEMS];
    SimpleMenuSection menu_sections[1];
    ClaySettings *settings;
};

SettingsMenu *SettingsMenu_init(ClaySettings *settings);

void SettingsMenu_destroy(SettingsMenu *settings_menu);

void SettingsMenu_open_menu(SettingsMenu *settings_menu);