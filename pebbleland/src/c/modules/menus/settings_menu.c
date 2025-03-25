#include "settings_menu.h"
#include "../defines.h"

void SettingsMenu_window_load(Window *window) {
    SettingsMenu *settings_menu = (SettingsMenu *) (window_get_user_data(window));

    SettingsMenu_open_menu(settings_menu);
}

void SettingsMenu_window_unload(Window *window) {
    SettingsMenu *settings_menu = (SettingsMenu *) (window_get_user_data(window));
    
    SettingsMenu_destroy(settings_menu);
    window_destroy(window);
}

SettingsMenu *SettingsMenu_init(ClaySettings *settings) { // TODO: add callback
    SettingsMenu *settings_menu = NULL;
    settings_menu = malloc(sizeof(SettingsMenu));
    if (settings_menu == NULL)
        return NULL;

    settings_menu->window = window_create();
    settings_menu->settings = settings;

    window_set_window_handlers(settings_menu->window, (WindowHandlers) {
      .load = SettingsMenu_window_load,
      .unload = SettingsMenu_window_unload
    });
    window_set_user_data(settings_menu->window, (void *) settings_menu);
    window_stack_push(settings_menu->window, true);
    
    return settings_menu;
}

void SettingsMenu_destroy(SettingsMenu *settings_menu) {
    if (settings_menu->menu_layer != NULL)
        simple_menu_layer_destroy(settings_menu->menu_layer);
    if (settings_menu != NULL) {
        free(settings_menu);
    }
}

void SettingsMenu_update_gyro(SettingsMenu *settings_menu) {
    settings_menu->menu_items[MENU_ITEM_GYRO].subtitle = settings_menu->settings->Gyro ? "Enabled" : "Disabled";
    // layer_mark_dirty(simple_menu_layer_get_layer(settings_menu->menu_layer));
    layer_mark_dirty(window_get_root_layer(settings_menu->window));
}

void SettingsMenu_callback(int index, void *ctx) {
    SettingsMenu *settings_menu = (SettingsMenu *) (ctx);
    if (index == MENU_ITEM_GYRO) {
        settings_menu->settings->Gyro = !settings_menu->settings->Gyro;
        save_settings(settings_menu->settings);
        SettingsMenu_update_gyro(settings_menu);
    }
}

void SettingsMenu_open_menu(SettingsMenu *settings_menu) {
    settings_menu->menu_items[MENU_ITEM_GYRO] = (SimpleMenuItem) {
        .title = "Tilt Controls",
        .callback = SettingsMenu_callback
    };
    SettingsMenu_update_gyro(settings_menu);

    settings_menu->menu_sections[0] = (SimpleMenuSection) {
        .num_items = SETTINGS_MENU_NUM_ITEMS,
        .items = settings_menu->menu_items,
    };

    GRect bounds = layer_get_bounds(window_get_root_layer(settings_menu->window));
    
    settings_menu->menu_layer = simple_menu_layer_create(bounds, settings_menu->window, settings_menu->menu_sections, 1, (void *) settings_menu);
    
    layer_add_child(window_get_root_layer(settings_menu->window), simple_menu_layer_get_layer(settings_menu->menu_layer));
}