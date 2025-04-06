#pragma once

#include <pebble.h>
#include "../defines.h"

typedef struct _message_queue MessageQueue;
struct _message_queue {
    char messages[MESSAGE_QUEUE_LEN][POPUP_TEXT_LEN];
    int head, tail;
};

MessageQueue *MessageQueue_init();
void MessageQueue_destroy(MessageQueue *message_queue);
bool MessageQueue_is_full(MessageQueue *message_queue);
bool MessageQueue_is_empty(MessageQueue *message_queue);
bool MessageQueue_push(MessageQueue *message_queue, char *text);
char *MessageQueue_pop(MessageQueue *message_queue);