import math
import os
import sys
import glob
from PIL import Image, ImageDraw
from Templates import BACKGROUND_TEMPLATE, SPRITE_TEMPLATE, RESOURCE_TEMPLATE
from distutils.dir_util import copy_tree
import shutil
import json

TILE_WIDTH = 8
TILE_HEIGHT = 8
IMAGE_TRANSFORMATIONS = [
    (False, False),
    (True, False),
    (False, True),
    (True, True)
]
COLORS_PER_PALETTE = 16
NUM_PALETTES = 8
NUMBERS = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":"]
NUMBER_WIDTH = 3
NUMBER_HEIGHT = 5
PALETTE_PIXEL_SIZE = 8

def colors_are_the_same(image_1, image_2):
    return list(image_1.convert('RGB').getdata()) == list(image_2.convert('RGB').getdata())

def alphas_are_the_same(image_1, image_2):
    return [i[-1] for i in image_1.convert('RGBA').getdata()] == [i[-1] for i in image_2.convert('RGBA').getdata()]

# Will return a tuple of:
# - index - the index of the image (None if not found)
# - x_flip - if the image is flipped in the x direction
# - y_flip - if the image is flipped in the y direction
def find_image(image_to_find, images_to_search):
    for x_flip, y_flip in IMAGE_TRANSFORMATIONS:
        flipped_image_to_find = image_to_find
        if x_flip:
            flipped_image_to_find = flipped_image_to_find.transpose(Image.FLIP_LEFT_RIGHT)
        if y_flip:
            flipped_image_to_find = flipped_image_to_find.transpose(Image.FLIP_TOP_BOTTOM)

        for image_index in range(len(images_to_search)):
            if colors_are_the_same(images_to_search[image_index], flipped_image_to_find) and alphas_are_the_same(images_to_search[image_index], flipped_image_to_find):
                return (image_index, x_flip, y_flip)
    
    return (None, None, None)

# Modifies tile_bank by adding the tile if it's unique (for unique_only)
def extract_tiles(image_filename, tile_bank, unique_only):
    image = Image.open(image_filename)
    dims = image.size
    
    for y in range(0, dims[1], TILE_HEIGHT):
        for x in range(0, dims[0], TILE_WIDTH):
            tile = image.crop((x, y, x + TILE_WIDTH, y + TILE_HEIGHT))
            if unique_only:
                image_index, _x_flip, _y_flip = find_image(tile, tile_bank)
                if image_index is None:
                    tile_bank.append(tile)
            else:
                tile_bank.append(tile)

def extract_palettes_from_images(images):
        palettes = []
        for i in range(NUM_PALETTES):
            palettes.append(set())
            palettes[-1].add((0, 0, 0, 0))
        for image in images:
            colors = image.convert("RGBA").getcolors()
            image_colors = set()
            for _count, color in colors:
                if color[3] == 0:
                    color = (0, 0, 0, 0)
                image_colors.add(color)
            
            for palette in palettes:
                if len(image_colors | palette) <= COLORS_PER_PALETTE:
                    palette |= image_colors
                    break
        return [sorted(list(p)) for p in palettes]

    
def assign_palettes_to_images(images, palettes):
    assignments = []
    
    for image in images:
        colors = image.convert("RGBA").getcolors()
        image_colors = set()
        for _count, color in colors:
            if color[3] == 0:
                color = (0, 0, 0, 0)
            image_colors.add(color)
        
        for i in range(len(palettes)):
            if image_colors.issubset(set(palettes[i])):
                assignments.append(i)
                break
    return assignments

def make_attribute(palette=0, vram_bank=0, x_flip=0, y_flip=0, hide=0):
    return palette | (vram_bank << 3) | (x_flip << 5) | (y_flip << 6) | (hide << 7)

def generate_tilemap_and_attrmap(image_filename, palette_assignments, tiles):
    tilemap = []
    attrmap = []
    image = Image.open(image_filename)
    dims = image.size

    for y in range(0, dims[1], TILE_HEIGHT):
        for x in range(0, dims[0], TILE_WIDTH):
            tile = image.crop((x, y, x + TILE_WIDTH, y + TILE_HEIGHT))
            image_index, x_flip, y_flip = find_image(tile, tiles)
            tilemap.append(image_index)
            attrmap.append(make_attribute(palette=palette_assignments[image_index], x_flip=x_flip, y_flip=y_flip))
    return tilemap, attrmap

def generate_number_maps(number_filename, tiles, palette_assignments):
    number_image = Image.open(number_filename)
    numbers = {}

    for n in range(len(NUMBERS)):
        number_tiles = []
        number_attrs = []
        x_start = TILE_WIDTH * NUMBER_WIDTH * n
        for y in range(0, TILE_HEIGHT * NUMBER_HEIGHT, TILE_HEIGHT):
            for x in range(x_start, x_start + TILE_WIDTH * NUMBER_WIDTH, TILE_WIDTH):
                tile = number_image.crop((x, y, x + TILE_WIDTH, y + TILE_HEIGHT))
                image_index, x_flip, y_flip = find_image(tile, tiles)
                number_tiles.append(image_index)
                number_attrs.append(make_attribute(palette=palette_assignments[image_index], x_flip=x_flip, y_flip=y_flip))
        numbers[NUMBERS[n]] = (number_tiles, number_attrs)
    return numbers

def generate_tilesheet(tiles, palettes, palette_assignments):
    tilesheet_bytestring = ""
    for tile, palette_num in zip(tiles, palette_assignments):
        tile_bytestring = ""
        for y in range(tile.size[1]):
            for x in range(tile.size[0]):
                pixel = tile.convert("RGBA").getpixel((x,y))
                if pixel[3] == 0:
                    pixel = (0, 0, 0, 0)

                tile_bytestring += format(palettes[palette_num].index(pixel), '04b')
        tilesheet_bytestring += tile_bytestring
    return int(tilesheet_bytestring, 2).to_bytes((len(tilesheet_bytestring) + 7) // 8, byteorder="big")

def write_tilesheet_to_file(tilesheet, filename):
    with open(filename, "wb") as f_out:
        f_out.write(tilesheet)
        
def write_map_to_file(map, filename):
    with open(filename, "wb") as f_out:
        f_out.write(bytes(map))

def write_number_maps_to_file(number_maps, filename):
    number_maps_list = []
    for number in NUMBERS:
        tiles, attrs = number_maps[number]
        for tile, attr in zip(tiles, attrs):
            number_maps_list += [tile, attr]
    
    with open(filename, "wb") as f_out:
        f_out.write(bytes(number_maps_list))
        
def convert_rgba_to_pebble_color(color):
    pebble_color = 0b11000000
    pebble_color += (color[0] // 85) << 4 # R
    pebble_color += (color[1] // 85) << 2 # G
    pebble_color += (color[2] // 85) << 0 # Bs
    return pebble_color

def write_palettes_to_file(palettes, filename):
    palette_array = []
    for palette in palettes:
        for i in range(COLORS_PER_PALETTE):
            if i >= len(palette):
                color = (0, 0, 0, 0)
            else:
                color = palette[i]
            palette_array.append(convert_rgba_to_pebble_color(color))
    with open(filename, "wb") as f_out:
        f_out.write(bytes(palette_array))

def save_tilesheet_as_image(tiles, filename):
        dims = (len(tiles) * TILE_WIDTH, TILE_HEIGHT)

        tilesheet = Image.new("RGBA", dims)
        for x in range(len(tiles)):
            tilesheet.paste(tiles[x], (x * TILE_WIDTH, 0))

        tilesheet.save(filename)
        
def save_palettes_as_image(palettes, filename):
    palette_img = Image.new('RGB', (COLORS_PER_PALETTE * PALETTE_PIXEL_SIZE, NUM_PALETTES * PALETTE_PIXEL_SIZE))
    palette_draw = ImageDraw.Draw(palette_img)
    for p in range(NUM_PALETTES):
        palette = palettes[p]
        for c in range(COLORS_PER_PALETTE):
            if c >= len(palette):
                color = (0, 0, 0, 0)
            else:
                color = palette[c]
            color = color[:3] # remove alpha

            start = (c * PALETTE_PIXEL_SIZE, p * PALETTE_PIXEL_SIZE)
            end = (c * PALETTE_PIXEL_SIZE + PALETTE_PIXEL_SIZE, p * PALETTE_PIXEL_SIZE + PALETTE_PIXEL_SIZE)
            palette_draw.rectangle((start, end), fill=color, width=0)
    palette_img.save(filename)

def add_resource_to_package(package, subdirectory, resource_name_info):
    # Resource name info: [group name, resource name, resource file type]
    resource = RESOURCE_TEMPLATE.copy()
    resource_location = subdirectory + "/" + resource_name_info[1] + resource_name_info[2]
    uppercase_resource_name = (resource_name_info[0] + "_" + resource_name_info[1]).upper()
    resource["name"] = resource["name"].format(**locals())
    resource["file"] = resource["file"].format(**locals())
    package["pebble"]["resources"]["media"].append(resource)

def add_resource_to_backgrounds_h(backgrounds_h_filename, background_group):
    with open(backgrounds_h_filename, "a") as backgrounds_h_file:
        uppercase_background_group = background_group.upper()
        background_loader = BACKGROUND_TEMPLATE.format(**locals())
        backgrounds_h_file.write(background_loader)

def add_loader_info_to_backgrounds_h(backgrounds_h_filename, background_groups):
    loader_info = f"#define NUM_BACKGROUND_GROUPS {len(background_groups)}\n\n"
    loader_info += f"uint16_t (*LOAD_BACKGROUND_GROUP[{len(background_groups)}])(GBC_Graphics *gbc_graphics, uint8_t *numbers_buffer) = {{\n"
    for background_group in background_groups:
        loader_info += f"    load_{background_group},\n"
    loader_info += "};\n"
    with open(backgrounds_h_filename, "a") as backgrounds_h_file:
        backgrounds_h_file.write(loader_info)
        
def extract_sprite_and_tiles(image_filename, tiles, sprites):
    image = Image.open(image_filename)
    dims = image.size

    sprites.append([
        os.path.splitext(os.path.basename(image_filename))[0], 
        len(tiles), 
        # width = TILE_SIZE * (2 ** sprite_width)
        # (width / tile_size) = 2 ** sprite_width
        # log2(width / tile_size) == sprite_width
        round(math.ceil(dims[0] / TILE_WIDTH) - 1), 
        round(math.ceil(dims[1] / TILE_HEIGHT) - 1)
    ])
    
    for y in range(0, dims[1], TILE_HEIGHT):
        for x in range(0, dims[0], TILE_WIDTH):
            tile = image.crop((x, y, x + TILE_WIDTH, y + TILE_HEIGHT))
            tiles.append(tile)
    
    return tiles, sprites

def add_resource_to_sprites_h(sprites_h_filename, sprite_group, num_sprites):
    with open(sprites_h_filename, "a") as sprites_h_file:
        uppercase_sprite_group = sprite_group.upper()
        sprite_loader = SPRITE_TEMPLATE.format(**locals())
        sprites_h_file.write(sprite_loader)
        
def add_loader_info_to_sprites_h(sprites_h_filename, sprite_groups):
    loader_info = f"#define NUM_SPRITE_GROUPS {len(sprite_groups)}\n\n"
    loader_info += f"uint16_t (*LOAD_SPRITE_GROUP[{len(sprite_groups)}])(GBC_Graphics *gbc_graphics, uint8_t vram_bank, uint8_t vram_start_offset, uint8_t palette_num, uint8_t *num_sprites, const uint8_t **sprite_data) = {{\n"
    for sprite_group in sprite_groups:
        loader_info += f"    load_{sprite_group},\n"
    loader_info += "};\n"
    with open(sprites_h_filename, "a") as sprites_h_file:
        sprites_h_file.write(loader_info)

def add_sprite_data_structure_to_sprites_h(sprites_h_filename, sprites, sprite_group):
    sprite_defines = ""
    for i in range(len(sprites)):
        sprite = sprites[i]
        sprite_defines += f"#define {sprite_group.upper()}_{sprite[0].upper()} {i}\n"
    sprite_defines += "\n"
    
    with open(sprites_h_filename, "a") as sprites_h_file:
        sprites_h_file.write(sprite_defines)
        
    sprite_data_structure = f"const uint8_t {sprite_group.upper()}_DATA[] = {{\n"
        # e.g.MarioBig_Walk = {tile, width, height}
    sprite_data_structure += "    // vram tile start, width, height\n"
    for i in range(len(sprites)):
        sprite = sprites[i]
        sprite_data_structure += f"    {sprite[1]}, {sprite[2]}, {sprite[3]}, // {sprite[0]}\n"
    sprite_data_structure += "};\n"

    with open(sprites_h_filename, "a") as sprites_h_file:
        sprites_h_file.write(sprite_data_structure)
        
def convert_rgba_to_pebble_color_string(color):
    pebble_color = "0b11" # A
    pebble_color += format(color[0] // 85, '02b') # R
    pebble_color += format(color[1] // 85, '02b') # G
    pebble_color += format(color[2] // 85, '02b') # B
    return pebble_color

def convert_rgba_to_bw_pebble_color_string(color):
    color_sum = 0
    color_sum += color[0] / 85 # R
    color_sum += color[1] / 85 # G
    color_sum += color[2] / 85 # B
    color_average = round(color_sum / 3)
    if color_average > 0:
        color_average = min(color_average + 1, 0b11)
    bw_pebble_color = '0b' + format(color_average, '02b')
    return bw_pebble_color

def add_palette_data_structure_to_palettes_h(palettes_h_filename, palettes, group_name, only_use_first_palette=False):
    palette_data_structure = f"uint8_t {group_name.upper()}_PALETTES[][{COLORS_PER_PALETTE}] = {{\n"
    palette_data_structure += f"#if defined(PBL_COLOR)\n"
    for palette in palettes:
        palette_data_structure += "    {"
        for i in range(COLORS_PER_PALETTE):
            if i >= len(palette):
                color = (0, 0, 0, 0)
            else:
                color = palette[i]
            palette_data_structure += f"{convert_rgba_to_pebble_color_string(color)}, "
        palette_data_structure += "},\n"
        if only_use_first_palette:
            break

    palette_data_structure += f"#else\n"
    for palette in palettes:
        palette_data_structure += "    {"
        for i in range(COLORS_PER_PALETTE):
            if i >= len(palette):
                color = (0, 0, 0, 0)
            else:
                color = palette[i]
            palette_data_structure += f"{convert_rgba_to_bw_pebble_color_string(color)}, "
        palette_data_structure += "},\n"
        if only_use_first_palette:
            break

    palette_data_structure += f"#endif\n"
    palette_data_structure += "};\n\n"
    
    with open(palettes_h_filename, "a") as palettes_h_file:
        palettes_h_file.write(palette_data_structure)

def update_defines_h(defines_h_filename, config):
    time_y_tile_offset = config["timeYTileOffset"]
    time_y_pixel_shift = config["timeYPixelShift"]
    sprite_bottom_y = config["spriteBottomY"]
    background_step_size = config["backgroundStepSize"]
    foreground_step_size = config["foregroundStepSize"]
    sprite_step_size = config["spriteStepSize"]
    update_every_second = config["updateEverySecond"]
    with open(defines_h_filename) as defines_file:
        old_defines = defines_file.read()
        new_defines = old_defines.format(**locals())

    with open(defines_h_filename, "w") as defines_file:
        defines_file.write(new_defines)

def convert(directory):
    # # Load config
    # print("Loading config")
    # config_filename = os.path.join(directory, "config.json")
    # with open(config_filename) as config_file:
    #     config = json.load(config_file)
    
    # Setup output directory
    print("Setting up output directory...")
    output_directory = os.path.join(directory, "Output")
    if os.path.exists(output_directory):
        shutil.rmtree(output_directory)
    os.makedirs(output_directory)
    output_reference_directory = os.path.join(output_directory, "References")
    os.makedirs(output_reference_directory)

    # # Duplicate the template project, update package.json
    # print("Duplicating template project...")
    # watchface_directory = os.path.join(output_directory, config['watchfaceName'])
    # os.makedirs(watchface_directory)
    # copy_tree("template-watchface", watchface_directory)
    # palettes_h_filename = os.path.join(watchface_directory, "src", "c", "resources", "Palettes.h")
    palettes_h_filename = os.path.join(output_directory, "palettes.h")

    # # Update package.json with config values
    # print("Replacing package values with config values...")
    # package_filename = os.path.join(watchface_directory, "package.json")
    # with open(package_filename) as package_file:
    #     package = json.load(package_file)
    # with open(package_filename, "w") as package_file:
    #     package["name"] = config["watchfaceName"]
    #     package["author"] = config["watchfaceAuthor"]
    #     package["pebble"]["displayName"] = config["watchfaceDisplayName"]
    #     package["pebble"]["uuid"] = config["uuid"]
    #     json.dump(package, package_file, indent=4)

    # Start with backgrounds
    # print(f"Processing backgrounds...")
    # background_directory = os.path.join(directory, "Backgrounds")
    # background_group_directories = glob.iglob(os.path.join(background_directory, "*"))
    # backgrounds_h_filename = os.path.join(watchface_directory, "src", "c", "resources", "Backgrounds.h")
    # background_groups = []
    # for background_group_directory in background_group_directories:
    #     background_group_name = os.path.basename(background_group_directory)
    #     print(f"Processing {background_group_name}...")

    #     background_filename = os.path.join(background_group_directory, "Background.png")
    #     foreground_filename = os.path.join(background_group_directory, "Foreground.png")
    #     numbers_filename = os.path.join(background_group_directory, "Numbers.png")

    #     # Extract tiles
    #     print("    Extracting background tiles...")
    #     background_group_tiles = []
    #     extract_tiles(background_filename, background_group_tiles, True)
    #     extract_tiles(foreground_filename, background_group_tiles, True)
    #     extract_tiles(numbers_filename, background_group_tiles, True)
    #     print(f"        Extracted {len(background_group_tiles)} background tiles.")
    #     assert len(background_group_tiles) <= 256, "    Too many tiles! Aborting..."

    #     # Extract and assign palettes
    #     print("    Extracting background palettes...")
    #     background_group_palettes = extract_palettes_from_images(background_group_tiles)
    #     background_group_palettes_assignments = assign_palettes_to_images(background_group_tiles, background_group_palettes)

    #     # Generate maps
    #     print("    Generating maps...")
    #     background_tilemap, background_attrmap = generate_tilemap_and_attrmap(background_filename, background_group_palettes_assignments, background_group_tiles)
    #     foreground_tilemap, foreground_attrmap = generate_tilemap_and_attrmap(foreground_filename, background_group_palettes_assignments, background_group_tiles)
    #     number_maps = generate_number_maps(numbers_filename, background_group_tiles, background_group_palettes_assignments)

    #     # Generate tilesheet
    #     print("    Generating tilesheet...")
    #     background_group_tilesheet = generate_tilesheet(background_group_tiles, background_group_palettes, background_group_palettes_assignments)

    #     # Export data files
    #     print("    Exporting data files...")
    #     background_group_data_subdirectory = f"data/backgrounds/{background_group_name}"
    #     background_group_data_directory = os.path.join(watchface_directory, "resources", background_group_data_subdirectory)
    #     os.makedirs(background_group_data_directory)
    #     background_group_reference_directory = os.path.join(output_reference_directory, background_group_name)
    #     os.makedirs(background_group_reference_directory)

    #     # Export tilesheet
    #     write_tilesheet_to_file(background_group_tilesheet, os.path.join(background_group_data_directory, "Tilesheet.bin"))
    #     save_tilesheet_as_image(background_group_tiles, os.path.join(background_group_reference_directory, "Tilesheet.png"))

    #     # Export tilemaps
    #     write_map_to_file(background_tilemap, os.path.join(background_group_data_directory, "BackgroundTilemap.bin"))
    #     write_map_to_file(foreground_tilemap, os.path.join(background_group_data_directory, "ForegroundTilemap.bin"))
        
    #     # Export attrmaps
    #     write_map_to_file(background_attrmap, os.path.join(background_group_data_directory, "BackgroundAttrmap.bin"))
    #     write_map_to_file(foreground_attrmap, os.path.join(background_group_data_directory, "ForegroundAttrmap.bin"))

    #     # Export number maps
    #     write_number_maps_to_file(number_maps, os.path.join(background_group_data_directory, "NumberMaps.bin"))
        
    #     # Export palettes
    #     add_palette_data_structure_to_palettes_h(palettes_h_filename, background_group_palettes, background_group_name)
    #     save_palettes_as_image(background_group_palettes, os.path.join(background_group_reference_directory, "Palettes.png"))

    #     # Modify package to import data files
    #     print("    Updating package...")
    #     with open(package_filename) as package_file:
    #         package = json.load(package_file)
    #     with open(package_filename, "w") as package_file:
    #         add_resource_to_package(package, background_group_data_subdirectory, [background_group_name, "Tilesheet", ".bin"])
    #         add_resource_to_package(package, background_group_data_subdirectory, [background_group_name, "BackgroundTilemap", ".bin"])
    #         add_resource_to_package(package, background_group_data_subdirectory, [background_group_name, "BackgroundAttrmap", ".bin"])
    #         add_resource_to_package(package, background_group_data_subdirectory, [background_group_name, "ForegroundTilemap", ".bin"])
    #         add_resource_to_package(package, background_group_data_subdirectory, [background_group_name, "ForegroundAttrmap", ".bin"])
    #         add_resource_to_package(package, background_group_data_subdirectory, [background_group_name, "NumberMaps", ".bin"])
    #         json.dump(package, package_file, indent=4)

    #     # Add background loader function
    #     print("    Updating Backgrounds.h...")
    #     add_resource_to_backgrounds_h(backgrounds_h_filename, background_group_name)
    #     background_groups.append(background_group_name)
    
    # add_loader_info_to_backgrounds_h(backgrounds_h_filename, background_groups)
    # print("Backgrounds complete!")

    # Now do sprites in a similar fashion
    print("Processing sprites...")
    # sprite_directory = os.path.join(directory, "Sprites")
    sprite_directory = os.path.join(directory, "sprites")
    sprite_group_directories = glob.iglob(os.path.join(sprite_directory, "*"))
    # sprites_h_filename = os.path.join(watchface_directory, "src", "c", "resources", "Sprites.h")
    sprites_h_filename = os.path.join(output_directory, "sprites.h")
    sprite_groups = []
    sprite_nums = []
    for sprite_group_directory in sprite_group_directories:
        sprite_group_name = os.path.basename(sprite_group_directory)
        print(f"Processing {sprite_group_name}...")

        sprite_image_filenames = glob.iglob(os.path.join(sprite_group_directory, "*"))
        sprite_images = [Image.open(image_filename) for image_filename in sprite_image_filenames]
        sprite_group_palettes = extract_palettes_from_images(sprite_images)
        sprite_group_image_palettes = assign_palettes_to_images(sprite_images, sprite_group_palettes)

        print("    Extracting tiles, sprites, and palettes...")
        sprite_num = 0
        sprite_group_tiles = []
        sprite_group_sprites = []
        sprite_group_palettes_assignments = []
        sprite_image_filenames = glob.iglob(os.path.join(sprite_group_directory, "*"))
        for sprite_image_filename in sprite_image_filenames:
            extract_sprite_and_tiles(sprite_image_filename, sprite_group_tiles, sprite_group_sprites)
            for i in range((2 ** sprite_group_sprites[-1][2]) * (2 ** sprite_group_sprites[-1][3])):
                sprite_group_palettes_assignments.append(sprite_group_image_palettes[sprite_num])
            sprite_num += 1
        print(f"        Extracted {len(sprite_group_tiles)} sprite tiles.")
        assert len(sprite_group_tiles) <= 256, "    Too many tiles! Aborting..."

        # Generate tilesheet
        print("    Generating tilesheet...")
        sprite_group_tilesheet = generate_tilesheet(sprite_group_tiles, sprite_group_palettes, sprite_group_palettes_assignments)

        # Export data files
        print("    Exporting data files...")
        sprite_group_data_subdirectory = f"data/sprite/{sprite_group_name}"
        sprite_group_data_directory = os.path.join(output_directory, "resources", sprite_group_data_subdirectory)
        os.makedirs(sprite_group_data_directory)
        sprite_group_reference_directory = os.path.join(output_reference_directory, sprite_group_name)
        os.makedirs(sprite_group_reference_directory)

        # Export tilesheet
        write_tilesheet_to_file(sprite_group_tilesheet, os.path.join(sprite_group_data_directory, "Tilesheet.bin"))
        save_tilesheet_as_image(sprite_group_tiles, os.path.join(sprite_group_reference_directory, "Tilesheet.png"))

        # Export palettes
        add_palette_data_structure_to_palettes_h(palettes_h_filename, sprite_group_palettes, sprite_group_name, only_use_first_palette=True)
        save_palettes_as_image(sprite_group_palettes, os.path.join(sprite_group_reference_directory, "Palettes.png"))

        # # Modify package to import data files
        # print("    Updating package...")
        # with open(package_filename) as package_file:
        #     package = json.load(package_file)
        # with open(package_filename, "w") as package_file:
        #     add_resource_to_package(package, sprite_group_data_subdirectory, [sprite_group_name, "Tilesheet", ".bin"])
        #     json.dump(package, package_file, indent=4)

        print("    Updating Sprites.h...")
        # Add sprite data structure
        add_sprite_data_structure_to_sprites_h(sprites_h_filename, sprite_group_sprites, sprite_group_name)

        # Add sprite loader function
        add_resource_to_sprites_h(sprites_h_filename, sprite_group_name, sprite_num)
        sprite_groups.append(sprite_group_name)

    add_loader_info_to_sprites_h(sprites_h_filename, sprite_groups)

    # # Add user defines
    # print("Updating Defines.h...")
    # defines_h_filename = os.path.join(watchface_directory, "src", "c", "resources", "Defines.h")
    # update_defines_h(defines_h_filename, config)

    # print("Watchface generated! Located at ", watchface_directory)
    print("Full output at ", output_directory)


if __name__ == "__main__":
    convert(sys.argv[1])