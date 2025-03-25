#include <pebble.h>
#include "modules/menus/text_window.h"
#include "modules/defines.h"
#include "modules/enums.h"
#include "modules/communication.h"
#include "modules/game.h"
#include "modules/settings.h"
#include "pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"

static Window *s_main_window;
static StatusBarLayer *s_status_bar;
static TextLayer *s_main_text_layer, *s_sub_text_layer;
static GBC_Graphics *s_gbc_graphics;
static Game *s_game;
static AppTimer *s_frame_timer;

static bool s_logged_in = false;
static bool s_connected = false;
static AppState s_state = S_LOGIN;

static int s_clicks = 0;

static ClaySettings s_settings;

// Save the s_settings to persistent storage

/* timer callback */
static void frame_timer_handle(void* context) {
  Game_step(s_game);
  s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
}

static void start_game() {
  Game_start(s_game);
  text_layer_set_text(s_sub_text_layer, "");
  text_layer_set_text(s_main_text_layer, "");
  // TODO: request current users
  app_timer_register(FRAME_DURATION, frame_timer_handle, NULL); 
  s_state = S_PLAY;
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Message
  Tuple *message_t = dict_find(iterator, MESSAGE_KEY_Message);
  if (message_t) {
    TextWindow_init(message_t->value->cstring);
  }

  if (s_state == S_LOGIN) {
    Tuple *login_success_t = dict_find(iterator, MESSAGE_KEY_LoginSuccessful);
    if (login_success_t) {
      if (login_success_t->value->int32 == 1) {
        Tuple *username_t = dict_find(iterator, MESSAGE_KEY_Username);
        if (username_t) {
          strcpy(s_settings.Username, username_t->value->cstring);
          save_settings(&s_settings);
          // static char welcome_message[40];
          // snprintf(welcome_message, 40, "Welcome, %s!", s_settings.Username);
          // text_layer_set_text(s_main_text_layer, welcome_message);
          // text_layer_set_text(s_sub_text_layer, "Press select to start");
          // APP_LOG(APP_LOG_LEVEL_DEBUG, welcome_message);
          start_game();
        }
      }
    }
  }
  
  Tuple *new_username_t = dict_find(iterator, MESSAGE_KEY_NewUsername);
  if (new_username_t) {
    strcpy(s_settings.Username, new_username_t->value->cstring);
  }
  
  // Tuple *clicks_t = dict_find(iterator, MESSAGE_KEY_Clicks);
  // if (clicks_t) {
  //   Tuple *source_t = dict_find(iterator, MESSAGE_KEY_Source);
  //   s_clicks = clicks_t->value->int32;
  //   static char clicks_message[40];
  //   if (source_t) {
  //     snprintf(clicks_message, 40, "Last click:\n\t%s\nClicks: %d", source_t->value->cstring, s_clicks);
  //   } else {
  //     snprintf(clicks_message, 40, "Last click:\n\tUNKNOWN\nClicks: %d", s_clicks);
  //   }
  //   text_layer_set_text(s_sub_text_layer, clicks_message);
  // }

  if (s_state == S_PLAY) {
    Tuple *user_connected_t = dict_find(iterator, MESSAGE_KEY_UserConnected);
    if (user_connected_t) {
      Tuple *username_t = dict_find(iterator, MESSAGE_KEY_Username);
      Tuple *player_x_t = dict_find(iterator, MESSAGE_KEY_PlayerX);
      Tuple *player_y_t = dict_find(iterator, MESSAGE_KEY_PlayerY);
      if (username_t && player_x_t && player_y_t) {
        Game_add_player(s_game, 
                        username_t->value->cstring,
                        player_x_t->value->int16,
                        player_y_t->value->int16);
      }
    }

    // add location handler
    Tuple *user_location_t = dict_find(iterator, MESSAGE_KEY_Location);
    if (user_location_t) {
      Tuple *username_t = dict_find(iterator, MESSAGE_KEY_Username);
      Tuple *player_x_t = dict_find(iterator, MESSAGE_KEY_PlayerX);
      Tuple *player_y_t = dict_find(iterator, MESSAGE_KEY_PlayerY);
      if (username_t && player_x_t && player_y_t) {
        Game_update_player(s_game, 
                           username_t->value->cstring,
                           player_x_t->value->int16,
                           player_y_t->value->int16);
      }
    }


    // ad disconnect handler
    Tuple *user_disconnected_t = dict_find(iterator, MESSAGE_KEY_UserDisconnected);
    if (user_disconnected_t) {
      Tuple *username_t = dict_find(iterator, MESSAGE_KEY_Username);
      if (username_t) {
        Game_remove_player(s_game, username_t->value->cstring);
      }
    }

  }

  save_settings(&s_settings);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_LOGIN) {
    connect(s_settings.Username);
  } else if (s_state == S_PLAY) {
    Game_select_handler(s_game);
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_PLAY) {
    Game_up_handler(s_game);
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_PLAY) {
    Game_down_handler(s_game);
  }
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_LOGIN) {
    window_stack_pop(true);
  } else if (s_state == S_PLAY) {
    Game_back_handler(s_game);
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

static void will_focus_handler(bool in_focus) {
  if (!in_focus) {
    // If a notification pops up while the timer is firing
    // very rapidly, it will crash the entire watch :)
    // Stopping the timer when a notification appears will
    // prevent this while also pausing the gameplay
    if (s_frame_timer != NULL) {
      app_timer_cancel(s_frame_timer);
    }
  } else {
    if (s_frame_timer != NULL) {
      s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
    }
  }
}


static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(s_main_window, GColorWhite);

  // GBC Graphics Layer
  // Note, this creator adds the layer to the window, meaning if it's fired
  // first then it will be on the bottom
  s_gbc_graphics = GBC_Graphics_ctor(s_main_window, NUM_VRAMS, NUM_BACKGROUNDS);
  GBC_Graphics_set_screen_bounds(s_gbc_graphics, SCREEN_BOUNDS);
  GBC_Graphics_lcdc_set_enabled(s_gbc_graphics, false);
  s_game = Game_init(s_gbc_graphics, s_main_window, &s_settings);

  // Main text
  s_main_text_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y + STATUS_BAR_LAYER_HEIGHT + 30, bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT));
  GFont main_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

  text_layer_set_overflow_mode(s_main_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_main_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_main_text_layer, main_font);
  text_layer_set_text(s_main_text_layer, "PEBBLELAND");
  text_layer_set_background_color(s_main_text_layer, GColorClear);

  // Sub text
  s_sub_text_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y + bounds.size.h - 60, bounds.size.w, 60));
  GFont sub_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);

  text_layer_set_overflow_mode(s_sub_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_sub_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_sub_text_layer, sub_font);
  text_layer_set_background_color(s_sub_text_layer, GColorClear);
  text_layer_set_text(s_sub_text_layer, "Press SELECT to log in");

  // Create the StatusBarLayer
  s_status_bar = status_bar_layer_create();

  // Set properties
  status_bar_layer_set_colors(s_status_bar, GColorBlack, GColorWhite);
  status_bar_layer_set_separator_mode(s_status_bar, 
                                              StatusBarLayerSeparatorModeNone);

  // Add to Window
  layer_add_child(window_layer, text_layer_get_layer(s_main_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_sub_text_layer));
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
}

static void main_window_unload(Window *window) {
  // Destroy the StatusBarLayer
  status_bar_layer_destroy(s_status_bar);
  text_layer_destroy(s_main_text_layer);
  text_layer_destroy(s_sub_text_layer);
  Game_destroy(s_game);
  GBC_Graphics_destroy(s_gbc_graphics);
  disconnect(s_settings.Username);
}

static void init() {
  load_settings(&s_settings);
  
  // setup window
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);

  // Register callbacks for inputs
  window_set_click_config_provider(s_main_window, click_config_provider);

  // Register focus callback 
  app_focus_service_subscribe(will_focus_handler);
  
  // Register callbacks for app messages
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_focus_service_subscribe(will_focus_handler);

  // Open AppMessage
  const int inbox_size = 1024; // maaaaybe overkill, but 128 isn't enough
  const int outbox_size = 1024;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}