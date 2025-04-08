#include "conf_window.h"

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    ConfWindow *conf_window = (ConfWindow *) context;
    
    conf_window->choose_callback(true, conf_window->context);
    window_stack_remove(conf_window->window, true);
  }
  
  static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    ConfWindow *conf_window = (ConfWindow *) context;
    
    conf_window->choose_callback(false, conf_window->context);
    window_stack_remove(conf_window->window, true);
  }

void ConfWindow_action_bar_callback(void *context) {
    ConfWindow *conf_window = (ConfWindow *) context;
    
    window_set_click_context(BUTTON_ID_UP, context);
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_click_handler);
    window_set_click_context(BUTTON_ID_DOWN, context);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_click_handler);
}

void ConfWindow_window_load(Window *window) {
    ConfWindow *conf_window = (ConfWindow *) (window_get_user_data(window));
    
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    GFont font = fonts_get_system_font(CONF_FONT);
    
    GRect text_max_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w - ACTION_BAR_WIDTH, bounds.size.h);
    GSize text_size = graphics_text_layout_get_content_size(conf_window->text, font, 
                    text_max_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter);
    GRect text_bounds = GRect(0 + CONF_TEXT_INSET, bounds.origin.x + bounds.size.h / 2 - text_size.h / 2, text_size.w - CONF_TEXT_INSET, text_size.h + 4); // TODO: get rid of this magic number to prevent text cutoff (aka text box too small)
    
    // Create the TextLayer
    conf_window->text_layer = text_layer_create(text_bounds);
    text_layer_set_overflow_mode(conf_window->text_layer, GTextOverflowModeWordWrap);
    text_layer_set_font(conf_window->text_layer, font);
    text_layer_set_text(conf_window->text_layer, conf_window->text);
    layer_add_child(window_layer, text_layer_get_layer(conf_window->text_layer));

    conf_window->yes_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_CHECK);
    conf_window->no_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_CROSS);

    conf_window->action_bar_layer = action_bar_layer_create();
    action_bar_layer_set_icon(conf_window->action_bar_layer, BUTTON_ID_UP, conf_window->yes_bitmap);
    action_bar_layer_set_icon(conf_window->action_bar_layer, BUTTON_ID_DOWN, conf_window->no_bitmap);
    action_bar_layer_set_context(conf_window->action_bar_layer, (void *) conf_window);
    action_bar_layer_set_click_config_provider(conf_window->action_bar_layer,
                                                ConfWindow_action_bar_callback);  
    action_bar_layer_add_to_window(conf_window->action_bar_layer, window);
}

void ConfWindow_window_unload(Window *window) {
    ConfWindow *conf_window = (ConfWindow *) (window_get_user_data(window));
    
    ConfWindow_destroy(conf_window);
    window_destroy(window);
}

ConfWindow *ConfWindow_init(char *text, void (*callback)(bool, void *), void *context) {
    ConfWindow *conf_window = NULL;
    conf_window = malloc(sizeof(ConfWindow));
    if (conf_window == NULL)
        return NULL;

    conf_window->window = window_create();
    strcpy(conf_window->text, text);
    conf_window->choose_callback = callback;
    conf_window->context = context;

    window_set_window_handlers(conf_window->window, (WindowHandlers) {
      .load = ConfWindow_window_load,
      .unload = ConfWindow_window_unload
    });
    window_set_user_data(conf_window->window, (void *) conf_window);
    window_stack_push(conf_window->window, true);
    
    return conf_window;
}

void ConfWindow_destroy(ConfWindow *conf_window) {
    if (conf_window->text_layer != NULL)
        text_layer_destroy(conf_window->text_layer);
    if (conf_window->action_bar_layer != NULL)
        action_bar_layer_destroy(conf_window->action_bar_layer);
    if (conf_window->yes_bitmap != NULL)
        gbitmap_destroy(conf_window->yes_bitmap);
    if (conf_window->no_bitmap != NULL)
        gbitmap_destroy(conf_window->no_bitmap);
    if (conf_window != NULL) {
        free(conf_window);
    }
}