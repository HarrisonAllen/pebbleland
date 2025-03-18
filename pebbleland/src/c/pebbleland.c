#include <pebble.h>
#include "modules/scroll_layer.h"
#include "modules/defines.h"
#include "modules/enums.h"

static Window *s_main_window;
static StatusBarLayer *s_status_bar;
static TextLayer *s_main_text_layer, *s_clicks_text_layer;

static bool s_logged_in = false;
static bool s_connected = false;
static AppState s_state = S_LOGIN;

static int s_clicks = 0;

// Define settings struct
typedef struct ClaySettings {
  char Username[USERNAME_MAX_LEN];
} ClaySettings;

static ClaySettings settings;

static void login() {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result == APP_MSG_OK) {
    // what to do
    dict_write_uint8(iter, MESSAGE_KEY_RequestLogin, 1);
    dict_write_cstring(iter, MESSAGE_KEY_Username, settings.Username);

    // Send the message
    result = app_message_outbox_send();
  }
}

static void connect() {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result == APP_MSG_OK) {
    // what to do
    dict_write_uint8(iter, MESSAGE_KEY_Connect, 1);
    dict_write_cstring(iter, MESSAGE_KEY_Username, settings.Username);

    // Send the message
    result = app_message_outbox_send();
  }
}

static void disconnect() {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result == APP_MSG_OK) {
    // what to do
    dict_write_uint8(iter, MESSAGE_KEY_Disconnect, 1);
    dict_write_cstring(iter, MESSAGE_KEY_Username, settings.Username);

    // Send the message
    result = app_message_outbox_send();
  }
}

static void click(char *button) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result == APP_MSG_OK) {
    // what to do
    dict_write_uint8(iter, MESSAGE_KEY_Click, 1);
    dict_write_cstring(iter, MESSAGE_KEY_Button, button);

    // Send the message
    result = app_message_outbox_send();
  }
}

// Save the settings to persistent storage
static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Message
  Tuple *message_t = dict_find(iterator, MESSAGE_KEY_Message);
  if (message_t) {
    create_scroll_window(message_t->value->cstring);
  }

  Tuple *login_success_t = dict_find(iterator, MESSAGE_KEY_LoginSuccessful);
  if (login_success_t) {
    if (login_success_t->value->int32 == 1) {
      Tuple *username_t = dict_find(iterator, MESSAGE_KEY_Username);
      if (username_t) {
        strcpy(settings.Username, username_t->value->cstring);
        static char welcome_message[40];
        snprintf(welcome_message, 40, "Welcome, %s!", settings.Username);
        text_layer_set_text(s_main_text_layer, welcome_message);
        APP_LOG(APP_LOG_LEVEL_DEBUG, welcome_message);
      }
      s_state = S_MAIN;
    }
  }

  Tuple *new_username_t = dict_find(iterator, MESSAGE_KEY_NewUsername);
  if (new_username_t) {
    strcpy(settings.Username, new_username_t->value->cstring);
  }
  
  Tuple *clicks_t = dict_find(iterator, MESSAGE_KEY_Clicks);
  if (clicks_t) {
    Tuple *source_t = dict_find(iterator, MESSAGE_KEY_Source);
    s_clicks = clicks_t->value->int32;
    static char clicks_message[40];
    if (source_t) {
      snprintf(clicks_message, 40, "Last click:\n\t%s\nClicks: %d", source_t->value->cstring, s_clicks);
    } else {
      snprintf(clicks_message, 40, "Last click:\n\tUNKNOWN\nClicks: %d", s_clicks);
    }
    text_layer_set_text(s_clicks_text_layer, clicks_message);
  }

  save_settings();
}

// Message failed to receive
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

// Message failed to send
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

// Message sent successfully
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_LOGIN) {
    connect();
  } else if (s_state == S_MAIN) {
    click("select");
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_MAIN) {
    click("up");
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_MAIN) {
    click("down");
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void default_settings() {  
  strcpy(settings.Username, ""); 
}

// Read settings from persistent storage
static void load_settings() {
  // Load the default settings
  default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Main text
  s_main_text_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y + STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT));
  GFont main_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

  text_layer_set_overflow_mode(s_main_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_main_text_layer, main_font);
  text_layer_set_text(s_main_text_layer, "Press select to login");

  // Clicks text
  s_clicks_text_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y + bounds.size.h - 90, bounds.size.w, 90));
  GFont clicks_font = fonts_get_system_font(FONT_KEY_GOTHIC_28);

  text_layer_set_overflow_mode(s_clicks_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_clicks_text_layer, clicks_font);
  // text_layer_set_text(s_clicks_text_layer, "Clicks: 0");

  // Create the StatusBarLayer
  s_status_bar = status_bar_layer_create();

  // Set properties
  status_bar_layer_set_colors(s_status_bar, GColorBlack, GColorWhite);
  status_bar_layer_set_separator_mode(s_status_bar, 
                                              StatusBarLayerSeparatorModeNone);

  // Add to Window
  layer_add_child(window_layer, text_layer_get_layer(s_main_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_clicks_text_layer));
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
}

static void main_window_unload(Window *window) {
  // Destroy the StatusBarLayer
  status_bar_layer_destroy(s_status_bar);
  text_layer_destroy(s_main_text_layer);
  text_layer_destroy(s_clicks_text_layer);
}

static void init() {
  load_settings();
  
  // setup window
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);

  // Register callbacks for inputs
  window_set_click_config_provider(s_main_window, click_config_provider);
  
  // Register callbacks for app messages
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 1024; // maaaaybe overkill, but 128 isn't enough
  const int outbox_size = 1024;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  disconnect();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}