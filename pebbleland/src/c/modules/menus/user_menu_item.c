#include "user_menu_item.h"

UserMenuItem* UserMenuItem_create(char *username, int days_since_login, void (*callback) (int, void*)) {
    UserMenuItem *user_menu_item = NULL;
    user_menu_item = malloc(sizeof(UserMenuItem));
    if (user_menu_item == NULL)
        return NULL;
    
    if (days_since_login < 0) {
        snprintf(user_menu_item->online_status, 40, "Online");
    } else {
        snprintf(user_menu_item->online_status, 40, "Offline for %d day%s", days_since_login, days_since_login == 1 ? "" : "s");
    }
    user_menu_item->menu_item = (SimpleMenuItem) {
        .title = username,
        .subtitle = user_menu_item->online_status,
        .callback = callback
    };
    return user_menu_item;
}

void UserMenuItem_destroy(UserMenuItem *user_menu_item) {
    if (user_menu_item != NULL) {
        free(user_menu_item);
    }
}