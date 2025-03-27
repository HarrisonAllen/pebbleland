#include "user_menu.h"
#include "../windows/text_window.h"

void UserMenu_window_load(Window *window) {
    UserMenu *user_menu = (UserMenu *) (window_get_user_data(window));

    UserMenu_open_menu(user_menu);
}

void UserMenu_window_unload(Window *window) {
    UserMenu *user_menu = (UserMenu *) (window_get_user_data(window));
    
    UserMenu_destroy(user_menu);
    window_destroy(window);
}

UserMenu *UserMenu_init(Player **players) { // TODO: add callback
    UserMenu *user_menu = NULL;
    user_menu = malloc(sizeof(UserMenu));
    if (user_menu == NULL)
        return NULL;

    user_menu->window = window_create();
    user_menu->players = players;

    window_set_window_handlers(user_menu->window, (WindowHandlers) {
      .load = UserMenu_window_load,
      .unload = UserMenu_window_unload
    });
    window_set_user_data(user_menu->window, (void *) user_menu);
    window_stack_push(user_menu->window, true);
    
    return user_menu;
}

void UserMenu_destroy(UserMenu *user_menu) {
    if (user_menu->menu_layer != NULL)
        simple_menu_layer_destroy(user_menu->menu_layer);
    if (user_menu != NULL) {
        free(user_menu);
    }
}

void UserMenu_callback(int index, void *ctx) {
    UserMenu *user_menu = (UserMenu *) (ctx);

    char text_buffer[60];
    snprintf(text_buffer, 60, "Player information for %s will go here", user_menu->menu_items[index].title);
    TextWindow_init(text_buffer);
    layer_mark_dirty(simple_menu_layer_get_layer(user_menu->menu_layer));
}

void UserMenu_open_menu(UserMenu *user_menu) {
    int menu_item_index = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (user_menu->players[i]->active) {
            user_menu->user_menu_items[menu_item_index] = UserMenuItem_create(user_menu->players[i]->username, -1, UserMenu_callback);
            user_menu->menu_items[menu_item_index] = user_menu->user_menu_items[menu_item_index]->menu_item;
            menu_item_index++;
        }
    }
    
    user_menu->menu_sections[0] = (SimpleMenuSection) {
        .title = "Online",
        .num_items = menu_item_index,
        .items = user_menu->menu_items,
    };

    user_menu->offline_placeholder_item[0] = (SimpleMenuItem) {
        .title = "No offline users",
        .subtitle = "...for now",
    };
    
    user_menu->menu_sections[1] = (SimpleMenuSection) {
        .title = "Offline",
        .num_items = 1,
        .items = user_menu->offline_placeholder_item,
    };
    

    GRect bounds = layer_get_bounds(window_get_root_layer(user_menu->window));
    
    user_menu->menu_layer = simple_menu_layer_create(bounds, user_menu->window, user_menu->menu_sections, 2, (void *) user_menu);
    
    layer_add_child(window_get_root_layer(user_menu->window), simple_menu_layer_get_layer(user_menu->menu_layer));
}
