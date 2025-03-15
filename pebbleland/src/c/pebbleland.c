#include <pebble.h>

static TextLayer *s_text_layer;
static ScrollLayer *s_scroll_layer;
static Window *s_main_window;
static StatusBarLayer *s_status_bar;

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect small_bounds = GRect(bounds.origin.x, bounds.origin.y + STATUS_BAR_LAYER_HEIGHT + bounds.size.h * 0 / 5, bounds.size.w, bounds.size.h - bounds.size.h * 0 / 5 - STATUS_BAR_LAYER_HEIGHT);
  
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  
  // Find the bounds of the scrolling text
  GRect shrinking_rect = GRect(small_bounds.origin.x, small_bounds.origin.y, small_bounds.size.w, 2000);
  char *text = "Example text that is really really really really really \
                                really really really really really really really really really really really \
                                really really really really really really really really really really really \
                                really really really really really really long";
  GSize text_size = graphics_text_layout_get_content_size(text, font, 
                  shrinking_rect, GTextOverflowModeWordWrap, GTextAlignmentLeft);
  GRect text_bounds = GRect(0, 0, small_bounds.size.w, text_size.h);
  
  // Create the TextLayer
  s_text_layer = text_layer_create(text_bounds);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_text_layer, font);
  text_layer_set_text(s_text_layer, text);
  
  // Create the ScrollLayer
  s_scroll_layer = scroll_layer_create(small_bounds);
  
  // Set the scrolling content size
  scroll_layer_set_content_size(s_scroll_layer, text_size);
  
  // Let the ScrollLayer receive click events
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  
  // Add the TextLayer as a child of the ScrollLayer
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));
  
  // Add the ScrollLayer as a child of the Window
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));

  // Create the StatusBarLayer
  s_status_bar = status_bar_layer_create();

  // Set properties
  status_bar_layer_set_colors(s_status_bar, GColorBlack, GColorWhite);
  status_bar_layer_set_separator_mode(s_status_bar, 
                                              StatusBarLayerSeparatorModeNone);

  // Add to Window
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
}

static void main_window_unload(Window *window) {
  // Destroy the ScrollLayer and TextLayer
  scroll_layer_destroy(s_scroll_layer);
  text_layer_destroy(s_text_layer);
  // Destroy the StatusBarLayer
  status_bar_layer_destroy(s_status_bar);
}

static void init() {
  
  // setup window
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}