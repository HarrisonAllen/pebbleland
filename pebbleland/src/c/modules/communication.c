#include "communication.h"

void login(char *username) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        // what to do
        dict_write_uint8(iter, MESSAGE_KEY_RequestLogin, 1);
        dict_write_cstring(iter, MESSAGE_KEY_Username, username);

        // Send the message
        result = app_message_outbox_send();
    }
}

void connect(char *username) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        // what to do
        dict_write_uint8(iter, MESSAGE_KEY_Connect, 1);
        dict_write_cstring(iter, MESSAGE_KEY_Username, username);

        // Send the message
        result = app_message_outbox_send();
    }
}

void disconnect(char *username) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        // what to do
        dict_write_uint8(iter, MESSAGE_KEY_Disconnect, 1);
        dict_write_cstring(iter, MESSAGE_KEY_Username, username);

        // Send the message
        result = app_message_outbox_send();
    }
}

void click(char *button) {
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

void broadcast_connect(int x, int y, bool poll) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        // what to do
        if (poll) {
            dict_write_uint8(iter, MESSAGE_KEY_Poll, 1);
        }
        dict_write_uint8(iter, MESSAGE_KEY_BroadcastConnect, 1);
        dict_write_int16(iter, MESSAGE_KEY_PlayerX, x);
        dict_write_int16(iter, MESSAGE_KEY_PlayerY, y);

        // Send the message
        result = app_message_outbox_send();
    }
}

void broadcast_position(int x, int y) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        // what to do
        dict_write_uint8(iter, MESSAGE_KEY_Location, 1);
        dict_write_int16(iter, MESSAGE_KEY_PlayerX, x);
        dict_write_int16(iter, MESSAGE_KEY_PlayerY, y);

        // Send the message
        result = app_message_outbox_send();
    }
}

void poll_users() {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        // what to do
        dict_write_uint8(iter, MESSAGE_KEY_Poll, 1);

        // Send the message
        result = app_message_outbox_send();
    }
}

// Message failed to receive
void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

// Message failed to send
void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

// Message sent successfully
void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
