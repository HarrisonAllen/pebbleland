#include "communication.h"

void login(char *username, char *password, char *email) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        dict_write_uint8(iter, MESSAGE_KEY_RequestLogin, 1);
        dict_write_cstring(iter, MESSAGE_KEY_Username, username);
        dict_write_cstring(iter, MESSAGE_KEY_Password, password);
        dict_write_cstring(iter, MESSAGE_KEY_Email, email);

        result = app_message_outbox_send();
    }
}

void connect() {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        dict_write_uint8(iter, MESSAGE_KEY_Connect, 1);

        result = app_message_outbox_send();
    }
}

void disconnect(char *username) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        dict_write_uint8(iter, MESSAGE_KEY_Disconnect, 1);
        dict_write_cstring(iter, MESSAGE_KEY_Username, username);

        result = app_message_outbox_send();
    }
}

void broadcast_connect(PlayerData player_data, bool poll) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        if (poll) {
            dict_write_uint8(iter, MESSAGE_KEY_Poll, 1);
        }
        dict_write_uint8(iter, MESSAGE_KEY_BroadcastConnect, 1);
        dict_write_int16(iter, MESSAGE_KEY_X, player_data.x);
        dict_write_int16(iter, MESSAGE_KEY_Y, player_data.y);
        dict_write_uint8(iter, MESSAGE_KEY_Dir, player_data.dir);
        dict_write_uint8(iter, MESSAGE_KEY_HairStyle, player_data.hair_style);
        dict_write_uint8(iter, MESSAGE_KEY_ShirtStyle, player_data.shirt_style);
        dict_write_uint8(iter, MESSAGE_KEY_PantsStyle, player_data.pants_style);
        dict_write_uint8(iter, MESSAGE_KEY_HairColor, player_data.hair_color);
        dict_write_uint8(iter, MESSAGE_KEY_ShirtColor, player_data.shirt_color);
        dict_write_uint8(iter, MESSAGE_KEY_PantsColor, player_data.pants_color);
        dict_write_uint8(iter, MESSAGE_KEY_ShoesColor, player_data.shoes_color);

        result = app_message_outbox_send();
    }
}

void broadcast_position(int x, int y, Direction dir) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        dict_write_uint8(iter, MESSAGE_KEY_Location, 1);
        dict_write_int16(iter, MESSAGE_KEY_X, x);
        dict_write_int16(iter, MESSAGE_KEY_Y, y);
        dict_write_uint8(iter, MESSAGE_KEY_Dir, dir);

        result = app_message_outbox_send();
    }
}

void broadcast_update(PlayerData player_data) {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        dict_write_uint8(iter, MESSAGE_KEY_Update, 1);
        dict_write_int16(iter, MESSAGE_KEY_X, player_data.x);
        dict_write_int16(iter, MESSAGE_KEY_Y, player_data.y);
        dict_write_uint8(iter, MESSAGE_KEY_Dir, player_data.dir);
        dict_write_uint8(iter, MESSAGE_KEY_HairStyle, player_data.hair_style);
        dict_write_uint8(iter, MESSAGE_KEY_ShirtStyle, player_data.shirt_style);
        dict_write_uint8(iter, MESSAGE_KEY_PantsStyle, player_data.pants_style);
        dict_write_uint8(iter, MESSAGE_KEY_HairColor, player_data.hair_color);
        dict_write_uint8(iter, MESSAGE_KEY_ShirtColor, player_data.shirt_color);
        dict_write_uint8(iter, MESSAGE_KEY_PantsColor, player_data.pants_color);
        dict_write_uint8(iter, MESSAGE_KEY_ShoesColor, player_data.shoes_color);

        result = app_message_outbox_send();
    }
}

void poll_users() {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);

    if (result == APP_MSG_OK) {
        dict_write_uint8(iter, MESSAGE_KEY_Poll, 1);

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
