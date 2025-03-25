#include "text_window.h"

void TextWindow_window_load(Window *window) {
  TextWindow *text_window = (TextWindow *) (window_get_user_data(window));

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
    
  GFont font = fonts_get_system_font(SCROLL_FONT);
  
  // Find the bounds of the scrolling text
  GRect text_max_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, 2000);
  GSize text_size = graphics_text_layout_get_content_size(text_window->text, font, 
                  text_max_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft);
  GRect text_bounds = GRect(0, 0, bounds.size.w, text_size.h + 8); // TODO: get rid of this magic number to prevent text cutoff (aka text box too small)
  
  // Create the TextLayer
  text_window->text_layer = text_layer_create(text_bounds);
  text_layer_set_overflow_mode(text_window->text_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(text_window->text_layer, font);
  text_layer_set_text(text_window->text_layer, text_window->text);
  
  // Create the ScrollLayer
  text_window->scroll_layer = scroll_layer_create(bounds);
  
  // Set the scrolling content size
  scroll_layer_set_content_size(text_window->scroll_layer, text_size);
  
  // Let the ScrollLayer receive click events
  scroll_layer_set_click_config_onto_window(text_window->scroll_layer, window);
  
  // Add the TextLayer as a child of the ScrollLayer
  scroll_layer_add_child(text_window->scroll_layer, text_layer_get_layer(text_window->text_layer));
  
    
  // Add to Window
  layer_add_child(window_layer, scroll_layer_get_layer(text_window->scroll_layer));
}

void TextWindow_window_unload(Window *window) {
  TextWindow *text_window = (TextWindow *) (window_get_user_data(window));

  TextWindow_destroy(text_window);
  window_destroy(window);
}

TextWindow *TextWindow_init(char *text) {
  TextWindow *text_window = NULL;
  text_window = malloc(sizeof(TextWindow));
  if (text_window == NULL)
      return NULL;
  strcpy(text_window->text, text);

  text_window->window = window_create();

  window_set_window_handlers(text_window->window, (WindowHandlers) {
    .load = TextWindow_window_load,
    .unload = TextWindow_window_unload
  });
  window_set_user_data(text_window->window, (void *) text_window);
  window_stack_push(text_window->window, true);
  
  return text_window;
}

void TextWindow_destroy(TextWindow *text_window) {
  scroll_layer_destroy(text_window->scroll_layer);
  text_layer_destroy(text_window->text_layer);
}