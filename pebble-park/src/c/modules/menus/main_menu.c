#include "main_menu.h"
#include "../defines.h"
#include "../windows/text_window.h"
#include "user_menu.h"
#include "settings_menu.h"
#include "../windows/conf_window.h"
#include "outfit_menu.h"

void MainMenu_window_load(Window *window) {
    MainMenu *main_menu = (MainMenu *) (window_get_user_data(window));

    MainMenu_open_menu(main_menu);
}

void MainMenu_window_unload(Window *window) {
    MainMenu *main_menu = (MainMenu *) (window_get_user_data(window));
    
    MainMenu_destroy(main_menu);
    window_destroy(window);
}

MainMenu *MainMenu_init(Player **players, ClaySettings *settings) { // TODO: add callback
    MainMenu *main_menu = NULL;
    main_menu = malloc(sizeof(MainMenu));
    if (main_menu == NULL)
        return NULL;

    main_menu->window = window_create();
    main_menu->players = players;
    main_menu->settings = settings;

    window_set_window_handlers(main_menu->window, (WindowHandlers) {
      .load = MainMenu_window_load,
      .unload = MainMenu_window_unload
    });
    window_set_user_data(main_menu->window, (void *) main_menu);
    window_stack_push(main_menu->window, true);
    
    return main_menu;
}

void MainMenu_destroy(MainMenu *main_menu) {
    if (main_menu->menu_layer != NULL)
        simple_menu_layer_destroy(main_menu->menu_layer);
    if (main_menu->user_menu_item != NULL)
        UserMenuItem_destroy(main_menu->user_menu_item);
    if (main_menu != NULL) {
        free(main_menu);
    }
}

void MainMenu_logout_callback(bool logout_chosen, void *context) {
    MainMenu *main_menu = (MainMenu *) (context);
    if (logout_chosen) {
        window_stack_pop_all(true);
    }
}

void MainMenu_callback(int index, void *context) {
    // TODO: use game callback instead
    MainMenu *main_menu = (MainMenu *) (context);
    if (index == MENU_ITEM_INFO) {
        OutfitMenu_init(main_menu->players[0]);
    } else if (index == MENU_ITEM_SETTINGS) {
        SettingsMenu_init(main_menu->settings);
    } else if (index == MENU_ITEM_USERS) {
        UserMenu_init(main_menu->players);
    } else if (index == MENU_ITEM_FRIEND_REQUESTS) {
        // TODO: pop up friend request menu
    } else if (index == MENU_ITEM_HELP) {
        // TODO: pop up help menu
    } else if (index == MENU_ITEM_LOGOUT) {
        ConfWindow_init("Are you sure you want to log out?", MainMenu_logout_callback, (void *) main_menu);
    }
}

void MainMenu_open_menu(MainMenu *main_menu) {
    main_menu->user_menu_item = UserMenuItem_create(main_menu->players[0]->username, -1, MainMenu_callback);
    main_menu->menu_items[MENU_ITEM_INFO] = main_menu->user_menu_item->menu_item;
    main_menu->menu_items[MENU_ITEM_SETTINGS] = (SimpleMenuItem) {
        .title = "Settings",
        .callback = MainMenu_callback
    };
    main_menu->menu_items[MENU_ITEM_USERS] = (SimpleMenuItem) {
        .title = "Users",
        .callback = MainMenu_callback
    };
    main_menu->menu_items[MENU_ITEM_FRIEND_REQUESTS] = (SimpleMenuItem) {
        .title = "Friend Requests",
        .subtitle = "2 new requests!",
        .callback = MainMenu_callback
    };
    main_menu->menu_items[MENU_ITEM_HELP] = (SimpleMenuItem) {
        .title = "Help",
        .callback = MainMenu_callback
    };
    main_menu->menu_items[MENU_ITEM_LOGOUT] = (SimpleMenuItem) {
        .title = "Log Out",
        .callback = MainMenu_callback
    };

    main_menu->menu_sections[0] = (SimpleMenuSection) {
        .num_items = MAIN_MENU_NUM_ITEMS,
        .items = main_menu->menu_items,
    };

    GRect bounds = layer_get_bounds(window_get_root_layer(main_menu->window));
    
    main_menu->menu_layer = simple_menu_layer_create(bounds, main_menu->window, main_menu->menu_sections, 1, (void *) main_menu);
    
    layer_add_child(window_get_root_layer(main_menu->window), simple_menu_layer_get_layer(main_menu->menu_layer));
}