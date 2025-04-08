#pragma once

#include <pebble.h>
#include "../defines.h"

typedef struct _conf_window ConfWindow;
struct _conf_window {
    Window *window;
    TextLayer *text_layer;
    ActionBarLayer *action_bar_layer;
    GBitmap *yes_bitmap, *no_bitmap;
    char text[CONF_TEXT_LEN];
    void (*choose_callback)(bool, void *);
    void *context;
};

ConfWindow *ConfWindow_init(char *text, void (*callback)(bool, void *), void *context);

void ConfWindow_destroy(ConfWindow *conf_window);

void ConfWindow_action_bar_callback(void *context);

void ConfWindow_window_load(Window *window);

void ConfWindow_window_unload(Window *window);

void ConfWindow_destroy(ConfWindow *conf_window);