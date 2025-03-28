#pragma once

#include <pebble.h>

void login(char *username);
void connect(char *username);
void disconnect(char *username);
void click(char *button);
void broadcast_connect(int x, int y, bool poll);
void broadcast_position(int x, int y);
void poll_users();

void inbox_dropped_callback(AppMessageResult reason, void *context);
void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
void outbox_sent_callback(DictionaryIterator *iterator, void *context);