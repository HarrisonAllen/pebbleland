#include "message_queue.h"

MessageQueue *MessageQueue_init() {
    MessageQueue *message_queue = NULL;
    message_queue = malloc(sizeof(MessageQueue));
    if (message_queue == NULL)
        return NULL;
    message_queue->head = 0;
    message_queue->tail = 0;
    return message_queue;
}

void MessageQueue_destroy(MessageQueue *message_queue) {
    if (message_queue != NULL) {
        free(message_queue);
    }
}
bool MessageQueue_is_full(MessageQueue *message_queue) {
    return (message_queue->tail + 1) % MESSAGE_QUEUE_LEN == message_queue->head;
}

bool MessageQueue_is_empty(MessageQueue *message_queue) {
    return message_queue->tail == message_queue->head;
}

bool MessageQueue_push(MessageQueue *message_queue, char *text) {
    if (MessageQueue_is_full(message_queue)) {
        return false;
    }
    strcpy(message_queue->messages[message_queue->tail], text);
    message_queue->tail = (message_queue->tail + 1) % MESSAGE_QUEUE_LEN;
    return true;
}

char *MessageQueue_pop(MessageQueue *message_queue) {
    if (MessageQueue_is_empty(message_queue)) {
        return NULL;
    }
    char *result = message_queue->messages[message_queue->head];
    message_queue->head = (message_queue->head + 1) % MESSAGE_QUEUE_LEN;
    return result;
}