#include "outfit_sub_menu.h"
#include "../defines.h"
#include "../utility.h"

void OutfitSubMenu_window_load(Window *window) {
    OutfitSubMenu *outfit_sub_menu = (OutfitSubMenu *) (window_get_user_data(window));

    OutfitSubMenu_open_menu(outfit_sub_menu);
}

void OutfitSubMenu_window_unload(Window *window) {
    OutfitSubMenu *outfit_sub_menu = (OutfitSubMenu *) (window_get_user_data(window));
    
    outfit_sub_menu->close_callback(outfit_sub_menu->context);    
    OutfitSubMenu_destroy(outfit_sub_menu);
    window_destroy(window);
}

OutfitSubMenu *OutfitSubMenu_init(void (*update_callback)(int, void*), void (*close_callback)(), void *context, Layer *sprite_layer, TextLayer *text_layer) {
    OutfitSubMenu *outfit_sub_menu = NULL;
    outfit_sub_menu = malloc(sizeof(OutfitSubMenu));
    if (outfit_sub_menu == NULL)
        return NULL;

    outfit_sub_menu->window = window_create();
    outfit_sub_menu->update_callback = update_callback;
    outfit_sub_menu->close_callback = close_callback;
    outfit_sub_menu->context = context;
    outfit_sub_menu->sprite_layer = sprite_layer;
    outfit_sub_menu->text_layer = text_layer;

    window_set_window_handlers(outfit_sub_menu->window, (WindowHandlers) {
      .load = OutfitSubMenu_window_load,
      .unload = OutfitSubMenu_window_unload
    });
    window_set_user_data(outfit_sub_menu->window, (void *) outfit_sub_menu);
    window_stack_push(outfit_sub_menu->window, false);
    
    return outfit_sub_menu;
}

void OutfitSubMenu_destroy(OutfitSubMenu *outfit_sub_menu) {
    if (outfit_sub_menu->menu_layer != NULL)
        simple_menu_layer_destroy(outfit_sub_menu->menu_layer);
    if (outfit_sub_menu != NULL) {
        free(outfit_sub_menu);
    }
}

void OutfitSubMenu_open_menu(OutfitSubMenu *outfit_sub_menu) {
    outfit_sub_menu->menu_items[SUB_MENU_ITEM_STYLE] = (SimpleMenuItem) {
        .title = "Style",
        .callback = outfit_sub_menu->update_callback
    };
    outfit_sub_menu->menu_items[SUB_MENU_ITEM_COLOR] = (SimpleMenuItem) {
        .title = "Color",
        .callback = outfit_sub_menu->update_callback
    };

    outfit_sub_menu->menu_sections[0] = (SimpleMenuSection) {
        .num_items = OUTFIT_SUB_MENU_NUM_ITEMS,
        .items = outfit_sub_menu->menu_items,
    };

    GRect bounds = layer_get_bounds(window_get_root_layer(outfit_sub_menu->window));
    GRect menu_bounds = GRect(bounds.origin.x + bounds.size.w / 2, bounds.origin.y + OUTFIT_TEXT_LAYER_HEIGHT, 
                              bounds.size.w / 2, bounds.size.h - OUTFIT_TEXT_LAYER_HEIGHT);
    
    outfit_sub_menu->menu_layer = simple_menu_layer_create(menu_bounds, outfit_sub_menu->window, outfit_sub_menu->menu_sections, 1, outfit_sub_menu->context);
    layer_add_child(window_get_root_layer(outfit_sub_menu->window), simple_menu_layer_get_layer(outfit_sub_menu->menu_layer));
    
    layer_add_child(window_get_root_layer(outfit_sub_menu->window), outfit_sub_menu->sprite_layer);

    layer_add_child(window_get_root_layer(outfit_sub_menu->window), text_layer_get_layer(outfit_sub_menu->text_layer));
}