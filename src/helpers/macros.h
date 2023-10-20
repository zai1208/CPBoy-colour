#pragma once

#include "../core/preferences.h"

#define XSTR(x)           STR(x)
#define STR(x)            #x

#define CPBOY_VERSION     "v0.2.0"

#define CAS_LCD_WIDTH     320 
#define CAS_LCD_HEIGHT    528 

#define MAX_FILENAME_LEN  200

#define MCS_DIRECTORY     "CPBoy"

#define DIRECTORY_MAIN     "\\fls0\\CPBoy\\"
#define DIRECTORY_ROM      DIRECTORY_MAIN "roms"
#define DIRECTORY_CONFIGS  DIRECTORY_MAIN "configs"
#define DIRECTORY_CARTRAM  DIRECTORY_MAIN "cartrams"
#define DIRECTORY_PALETTES DIRECTORY_MAIN "palettes"

#define FILENAME_CONTROLS "controls.ccon"

#define EXTENSION_ROM     ".gb"
#define EXTENSION_CARTRAM ".csav"
#define EXTENSION_CONFIG  ".ini"
#define EXTENSION_PALETTE ".gbcp"

#define FILESIZE_CONFIG sizeof(rom_config)

#define TOGGLE(value) ((value) = !(value))

#define RGB555_TO_RGB565(rgb555) ( \
	0 | \
	((rgb555 & 0b0111110000000000) <<1) | \
	((rgb555 & 0b0000001111100000) <<1) | \
	(rgb555 & 0b0000000000011111) \
)
