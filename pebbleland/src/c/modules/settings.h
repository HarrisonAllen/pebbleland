#pragma once
#include <pebble.h>
#include "defines.h"

// Define settings struct
typedef struct ClaySettings {
    char Username[USERNAME_MAX_LEN];
    bool Tilt;
} ClaySettings;


static void save_settings(ClaySettings *settings) {
    persist_write_data(SETTINGS_KEY, settings, sizeof(*settings));
}

static void default_settings(ClaySettings *settings) {  
    strcpy(settings->Username, ""); 
    settings->Tilt = false;
}

// Read settings from persistent storage
static void load_settings(ClaySettings *settings) {
    // Load the default settings
    default_settings(settings);
    // Read settings from persistent storage, if they exist
    persist_read_data(SETTINGS_KEY, settings, sizeof(*settings));
  }