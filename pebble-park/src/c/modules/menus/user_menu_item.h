#pragma once
#include <pebble.h>

typedef struct _user_menu_item UserMenuItem;
struct _user_menu_item {
    SimpleMenuItem menu_item;
    char online_status[40];
};

UserMenuItem* UserMenuItem_create(char *username, int days_since_login, void (*callback) (int, void*));

void UserMenuItem_destroy(UserMenuItem *user_menu_item);