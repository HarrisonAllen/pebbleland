#include "main_menu.h"
#include "../defines.h"
#include "text_window.h"

void MainMenu_window_load(Window *window) {
    MainMenu *main_menu = (MainMenu *) (window_get_user_data(window));

    MainMenu_open_menu(main_menu);
}

void MainMenu_window_unload(Window *window) {
    MainMenu *main_menu = (MainMenu *) (window_get_user_data(window));
    
    MainMenu_destroy(main_menu);
    window_destroy(window);
}

MainMenu *MainMenu_init() { // TODO: add callback
    MainMenu *main_menu = NULL;
    main_menu = malloc(sizeof(MainMenu));
    if (main_menu == NULL)
        return NULL;

    main_menu->window = window_create();

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
    if (main_menu != NULL) {
        free(main_menu);
    }
}

void MainMenu_callback(int index, void *ctx) {
    // TODO: use game callback instead
    MainMenu *main_menu = (MainMenu *) (ctx);
    if (index == MENU_ITEM_INFO) {
        main_menu->menu_items[MENU_ITEM_INFO].subtitle = "No info yet...";
        TextWindow_init("No info yet...");
        layer_mark_dirty(simple_menu_layer_get_layer(main_menu->menu_layer));
    } else if (index == MENU_ITEM_SETTINGS) {
        main_menu->menu_items[MENU_ITEM_SETTINGS].subtitle = "No settings yet...";
        TextWindow_init("No settings yet...");
        layer_mark_dirty(simple_menu_layer_get_layer(main_menu->menu_layer));
    } else if (index == MENU_ITEM_USERS) {
        main_menu->menu_items[MENU_ITEM_USERS].subtitle = "No users yet...";
        TextWindow_init("No users yet...");
        layer_mark_dirty(simple_menu_layer_get_layer(main_menu->menu_layer));
    } else if (index == MENU_ITEM_LOGOUT) {
        window_stack_pop_all(true);
    }
}

void MainMenu_open_menu(MainMenu *main_menu) {
    main_menu->menu_items[MENU_ITEM_INFO] = (SimpleMenuItem) {
        .title = "Player Info",
        .callback = MainMenu_callback
    };
    main_menu->menu_items[MENU_ITEM_SETTINGS] = (SimpleMenuItem) {
        .title = "Settings",
        .callback = MainMenu_callback
    };
    main_menu->menu_items[MENU_ITEM_USERS] = (SimpleMenuItem) {
        .title = "Users",
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