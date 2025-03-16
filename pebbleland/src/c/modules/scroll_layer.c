#include "scroll_layer.h"
#include "defines.h"

static TextLayer *s_scroll_text_layer;
static ScrollLayer *s_scroll_layer;
static Window *s_scroll_window;
static StatusBarLayer *s_scroll_status_bar;
static char s_scroll_text[SCROLL_TEXT_LEN];

static void scroll_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  GRect scroll_bounds = GRect(bounds.origin.x, bounds.origin.y + STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT);
  
  GFont font = fonts_get_system_font(SCROLL_FONT);
  
  // Find the bounds of the scrolling text
  GRect text_max_bounds = GRect(scroll_bounds.origin.x, scroll_bounds.origin.y, scroll_bounds.size.w, 2000);
  GSize text_size = graphics_text_layout_get_content_size(s_scroll_text, font, 
                  text_max_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft);
  GRect text_bounds = GRect(0, 0, scroll_bounds.size.w, text_size.h);
  
  // Create the TextLayer
  s_scroll_text_layer = text_layer_create(text_bounds);
  text_layer_set_overflow_mode(s_scroll_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_scroll_text_layer, font);
  text_layer_set_text(s_scroll_text_layer, s_scroll_text);
  
  // Create the ScrollLayer
  s_scroll_layer = scroll_layer_create(scroll_bounds);
  
  // Set the scrolling content size
  scroll_layer_set_content_size(s_scroll_layer, text_size);
  
  // Let the ScrollLayer receive click events
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  
  // Add the TextLayer as a child of the ScrollLayer
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_scroll_text_layer));
  
  // Add the ScrollLayer as a child of the Window
  
  // Create the StatusBarLayer
  s_scroll_status_bar = status_bar_layer_create();
  
  // Set properties
  status_bar_layer_set_colors(s_scroll_status_bar, GColorBlack, GColorWhite);
  status_bar_layer_set_separator_mode(s_scroll_status_bar, 
    StatusBarLayerSeparatorModeNone);
    
    // Add to Window
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
  layer_add_child(window_layer, status_bar_layer_get_layer(s_scroll_status_bar));
}

static void scroll_window_unload(Window *window) {
  scroll_layer_destroy(s_scroll_layer);
  text_layer_destroy(s_scroll_text_layer);
  status_bar_layer_destroy(s_scroll_status_bar);
  window_destroy(s_scroll_window);

  s_scroll_layer = NULL;
  s_scroll_text_layer = NULL;
  s_scroll_status_bar = NULL;
  s_scroll_window = NULL;
}

void create_scroll_window(char *text) {
  strcpy(s_scroll_text, text); // This will be null terminated, right?
  // setup window
  s_scroll_window = window_create();

  window_set_window_handlers(s_scroll_window, (WindowHandlers) {
    .load = scroll_window_load,
    .unload = scroll_window_unload
  });
  window_stack_push(s_scroll_window, true);
}

// static void deinit() {
//   window_destroy(s_scroll_window);
// }