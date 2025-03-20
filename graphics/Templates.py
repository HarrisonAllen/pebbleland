BACKGROUND_TEMPLATE = \
"""
static uint16_t load_{background_group}(GBC_Graphics *gbc_graphics, uint8_t *numbers_buffer) {{
    // Load {background_group} tilesheet
    uint16_t loaded_tiles = load_tilesheet(gbc_graphics, RESOURCE_ID_DATA_{uppercase_background_group}_TILESHEET, 0, 0, 0);

    // Load {background_group} palettes
    for (uint8_t p = 0; p < GBC_PALETTE_NUM_PALETTES; p++) {{
        GBC_Graphics_set_bg_palette_array(gbc_graphics, p, {uppercase_background_group}_PALETTES[p]);
    }}

    // Load {background_group} tilemaps and attrmaps
    load_tilemap(gbc_graphics, RESOURCE_ID_DATA_{uppercase_background_group}_BACKGROUNDTILEMAP, BG_LAYER);
    load_attrmap(gbc_graphics, RESOURCE_ID_DATA_{uppercase_background_group}_BACKGROUNDATTRMAP, BG_LAYER);
    load_tilemap(gbc_graphics, RESOURCE_ID_DATA_{uppercase_background_group}_FOREGROUNDTILEMAP, FG_LAYER);
    load_attrmap(gbc_graphics, RESOURCE_ID_DATA_{uppercase_background_group}_FOREGROUNDATTRMAP, FG_LAYER);
    
    // Load {background_group} numbers
    load_resource(gbc_graphics, RESOURCE_ID_DATA_{uppercase_background_group}_NUMBERMAPS, numbers_buffer);
    return loaded_tiles;
}}

"""

SPRITE_TEMPLATE = \
"""
static uint16_t load_{sprite_group}(GBC_Graphics *gbc_graphics, uint8_t vram_bank, uint8_t vram_start_offset, uint8_t palette_num, uint8_t *num_sprites, const uint8_t **sprite_data) {{
    // Load {sprite_group} tilesheet
    uint16_t num_loaded_tiles = load_tilesheet(gbc_graphics, RESOURCE_ID_DATA_{uppercase_sprite_group}_TILESHEET, 0, vram_start_offset, vram_bank);

    // Load {sprite_group} palette
    GBC_Graphics_set_sprite_palette_array(gbc_graphics, palette_num, {uppercase_sprite_group}_PALETTES[0]);

    // Set number of sprites
    *num_sprites = {num_sprites};

    // Set sprite data
    *sprite_data = {uppercase_sprite_group}_DATA;
    
    return num_loaded_tiles;
}}

"""

RESOURCE_TEMPLATE = {
    "type": "raw",
    "name": "DATA_{uppercase_resource_name}",
    "file": "{resource_location}"
}