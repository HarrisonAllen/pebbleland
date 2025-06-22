#include "outfit_menu.h"
#include "../defines.h"
#include "../sprites/sprite_renderer.h"
#include "../utility.h"

void OutfitMenu_window_load(Window *window) {
    OutfitMenu *outfit_menu = (OutfitMenu *) (window_get_user_data(window));

    OutfitMenu_open_menu(outfit_menu);
}

void OutfitMenu_window_unload(Window *window) {
    OutfitMenu *outfit_menu = (OutfitMenu *) (window_get_user_data(window));
    
    OutfitMenu_destroy(outfit_menu);
    window_destroy(window);
}

OutfitMenu *OutfitMenu_init(Player *player) {
    OutfitMenu *outfit_menu = NULL;
    outfit_menu = malloc(sizeof(OutfitMenu));
    if (outfit_menu == NULL)
        return NULL;

    outfit_menu->window = window_create();
    outfit_menu->player = player;
    outfit_menu->hair = player->hair;
    outfit_menu->shirt = player->shirt;
    outfit_menu->pants = player->pants;
    outfit_menu->colors[0] = player->hair_color;
    outfit_menu->colors[1] = player->shirt_color;
    outfit_menu->colors[2] = player->pants_color;
    outfit_menu->colors[3] = player->shoe_color;

    window_set_window_handlers(outfit_menu->window, (WindowHandlers) {
      .load = OutfitMenu_window_load,
      .unload = OutfitMenu_window_unload
    });
    window_set_user_data(outfit_menu->window, (void *) outfit_menu);
    window_stack_push(outfit_menu->window, true);
    
    return outfit_menu;
}

void OutfitMenu_destroy(OutfitMenu *outfit_menu) {
    if (outfit_menu->menu_layer != NULL)
        simple_menu_layer_destroy(outfit_menu->menu_layer);
    if (outfit_menu != NULL) {
        free(outfit_menu);
    }
    if (outfit_menu->sprite_layer != NULL) {
        layer_destroy(outfit_menu->sprite_layer);
    }
}

void OutfitMenu_callback(int index, void *ctx) {
    OutfitMenu *outfit_menu = (OutfitMenu *) (ctx);
    if (index == MENU_ITEM_HAIR) {
        outfit_menu->hair = (outfit_menu->hair + 1) % HAIR_COUNT;
        outfit_menu->colors[0] = (outfit_menu->colors[0] + 1) % NUM_COLORS;
        layer_mark_dirty(outfit_menu->sprite_layer);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Hair select");
    }
    if (index == MENU_ITEM_SHIRT) {
        outfit_menu->shirt = (outfit_menu->shirt + 1) % CLOTHES_NUM_SHIRTS;
        outfit_menu->colors[1] = (outfit_menu->colors[1] + 1) % NUM_COLORS;
        layer_mark_dirty(outfit_menu->sprite_layer);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Shirt select");
    }
    if (index == MENU_ITEM_PANTS) {
        outfit_menu->pants = (outfit_menu->pants + 1) % CLOTHES_NUM_PANTS;
        outfit_menu->colors[2] = (outfit_menu->colors[2] + 1) % NUM_COLORS;
        layer_mark_dirty(outfit_menu->sprite_layer);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Pants select");
        // final shirt pants style is (shirt_style + pants_style * CLOTHES_NUM_SHIRTS)
    }
    if (index == MENU_ITEM_SHOES) {
        outfit_menu->colors[3] = (outfit_menu->colors[3] + 1) % NUM_COLORS;
        layer_mark_dirty(outfit_menu->sprite_layer);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Shoes select");
        // Only do color
    }
    if (index == MENU_ITEM_SAVE_OUTFIT) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Save outfit");
        Player_update_outfit(outfit_menu->player, outfit_menu->hair, outfit_menu->shirt, outfit_menu->pants, outfit_menu->colors);
        // Back exits without saving
        // Do a post here
        window_stack_pop(true);
    }
}

void OutfitMenu_sprite_viewer_update_proc(Layer *layer, GContext *ctx) {
    OutfitMenu * outfit_menu = *(OutfitMenu * *)layer_get_data(layer);
    GRect layer_bounds = layer_get_bounds(layer);
    GPoint pos = GPoint((layer_bounds.size.w - PREVIEW_OUTFIT_WIDTH) / 2,
                        (layer_bounds.size.h - PREVIEW_OUTFIT_HEIGHT) / 2);

    uint8_t hair_tile = HAIR_VRAM_START + outfit_menu->hair * HAIR_TILES_PER_HAIR;

    uint8_t clothes_tile = CLOTHES_VRAM_START + make_clothes_sprite_offset(outfit_menu->shirt, outfit_menu->pants);
    uint8_t palette[GBC_PALETTE_NUM_BYTES];
    create_player_palette(outfit_menu->colors, palette);
    render_outfit(ctx, outfit_menu->player->graphics, hair_tile, clothes_tile,
                  palette, pos, PREVIEW_OUTFIT_PIXEL_SIZE);
}

void OutfitMenu_open_menu(OutfitMenu *outfit_menu) {
    outfit_menu->menu_items[MENU_ITEM_HAIR] = (SimpleMenuItem) {
        .title = "Hair",
        .callback = OutfitMenu_callback
    };
    outfit_menu->menu_items[MENU_ITEM_SHIRT] = (SimpleMenuItem) {
        .title = "Shirt",
        .callback = OutfitMenu_callback
    };
    outfit_menu->menu_items[MENU_ITEM_PANTS] = (SimpleMenuItem) {
        .title = "Pants",
        .callback = OutfitMenu_callback
    };
    outfit_menu->menu_items[MENU_ITEM_SHOES] = (SimpleMenuItem) {
        .title = "Shoes",
        .callback = OutfitMenu_callback
    };
    outfit_menu->menu_items[MENU_ITEM_SAVE_OUTFIT] = (SimpleMenuItem) {
        .title = "Save",
        .callback = OutfitMenu_callback
    };
    // And then sub menus are like
    // HAIR
    // - Style
    // - Color
    // and repeat for rest

    outfit_menu->menu_sections[0] = (SimpleMenuSection) {
        .num_items = OUTFIT_MENU_NUM_ITEMS,
        .items = outfit_menu->menu_items,
    };

    GRect bounds = layer_get_bounds(window_get_root_layer(outfit_menu->window));
    GRect menu_bounds = GRect(bounds.origin.x + bounds.size.w / 2, bounds.origin.y + OUTFIT_TEXT_LAYER_HEIGHT, 
                              bounds.size.w / 2, bounds.size.h - OUTFIT_TEXT_LAYER_HEIGHT);
    
    outfit_menu->menu_layer = simple_menu_layer_create(menu_bounds, outfit_menu->window, outfit_menu->menu_sections, 1, (void *) outfit_menu);
    
    layer_add_child(window_get_root_layer(outfit_menu->window), simple_menu_layer_get_layer(outfit_menu->menu_layer));

    GRect sprite_bounds = GRect(bounds.origin.x, bounds.origin.y, 
                                bounds.size.w / 2, bounds.size.h);
    outfit_menu->sprite_layer = layer_create_with_data(sprite_bounds, sizeof(outfit_menu));
    *(OutfitMenu * *)layer_get_data(outfit_menu->sprite_layer) = outfit_menu;
    layer_set_update_proc(outfit_menu->sprite_layer, OutfitMenu_sprite_viewer_update_proc);
    layer_add_child(window_get_root_layer(outfit_menu->window), outfit_menu->sprite_layer);
    
    outfit_menu->text_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, OUTFIT_TEXT_LAYER_HEIGHT));
    GFont font = fonts_get_system_font(OUTFIT_TEXT_LAYER_FONT);

    text_layer_set_text_alignment(outfit_menu->text_layer, GTextAlignmentCenter);
    text_layer_set_font(outfit_menu->text_layer, font);
    text_layer_set_text(outfit_menu->text_layer, "Outfit Customization");
    text_layer_set_background_color(outfit_menu->text_layer, GColorWhite);
    text_layer_set_text_color(outfit_menu->text_layer, GColorBlack);
    layer_add_child(window_get_root_layer(outfit_menu->window), text_layer_get_layer(outfit_menu->text_layer));
}