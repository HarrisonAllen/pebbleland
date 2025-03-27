#pragma once

#include <pebble.h>
#include "../defines.h"

// Slide in, press back to dismiss

typedef struct _slide_layer SlideLayer;
struct _slide_layer {
    Window *window;
    Layer *layer;
    Animation *animation_sequence;
    char text[POPUP_TEXT_LEN];
    bool small;
    void (*destroy_callback)();
    void *destroy_context;
};

SlideLayer *SlideLayer_init(Window *window, char *text, bool small, void (*destroy_callback)(), void *destroy_context);

void SlideLayer_cancel(SlideLayer *slide_layer);

void SlideLayer_destroy(SlideLayer *slide_layer);