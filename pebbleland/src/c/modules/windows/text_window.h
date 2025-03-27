#pragma once

#include <pebble.h>
#include "../defines.h"

typedef struct _text_window TextWindow;
struct _text_window {
    Window *window;
    TextLayer *text_layer;
    ScrollLayer *scroll_layer;
    char text[SCROLL_TEXT_LEN];
};

TextWindow *TextWindow_init(char *text);

void TextWindow_destroy(TextWindow *text_window);