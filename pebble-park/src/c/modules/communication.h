#pragma once

#include <pebble.h>
#include "data.h"

void login(char *username);
void connect(char *username);
void disconnect(char *username);
void broadcast_connect(PlayerData player_data, bool poll);
void broadcast_position(int x, int y, Direction dir);
void broadcast_update(PlayerData player_data);
void poll_users();

void inbox_dropped_callback(AppMessageResult reason, void *context);
void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
void outbox_sent_callback(DictionaryIterator *iterator, void *context);