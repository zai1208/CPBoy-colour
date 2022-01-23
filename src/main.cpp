/**
 * MIT License
 *
 * Copyright (c) 2021 Sidney Krombholz 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stddef.h>
#include <appdef.hpp>
#include <sdk/os/file.hpp>
#include <sdk/os/debug.hpp>
#include <sdk/os/lcd.hpp>
#include <sdk/os/input.hpp>
#include <sdk/calc/calc.hpp>
#include <sdk/os/string.hpp>
#include <sdk/os/mem.hpp>
#include "UI/UI.hpp"
#include "peanut_gb.h"

// #define MAX_ROM_SIZE 0x10000

#define CP_KEY_UP				0
#define CP_KEY_DOWN			1
#define CP_KEY_LEFT			2
#define CP_KEY_RIGHT			3
#define CP_KEY_PLUS			4
#define CP_KEY_SHIFT			5
#define CP_KEY_CLEAR			6
#define CP_KEY_EXE				7
#define CP_KEY_KEYBOARD	8		
#define CP_KEY_BACKSPACE	9			
#define CP_KEY_NEGATIVE	10		

#define TAB_INFO 				0
#define TAB_SETTINGS 		1
#define TAB_LOAD_ROM 		2
#define TAB_SAVESTATES 	3

#define RGB555_TO_RGB565(rgb555) ( \
	0 | \
	((rgb555 & 0b0111110000000000) <<1) | \
	((rgb555 & 0b0000001111100000) <<1) | \
	(rgb555 & 0b0000000000011111) \
)

/*
 * Fill this section in with some information about your app.
 * All fields are optional - so if you don't need one, take it out.
 */
APP_NAME("CPBoy")
APP_DESCRIPTION("A Gameboy (DMG) emulator. Forked from PeanutGB by deltabeard.")
APP_AUTHOR("diddyholz")
APP_VERSION("0.0.2-alpha")

uint8_t *read_rom_to_ram(const char *file_name);
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr);
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr);
uint8_t read_cart_ram_file(const char *save_file_name, uint8_t **dest,
			const size_t len);
uint8_t write_cart_ram_file(const char *save_file_name, uint8_t **dest,
			const size_t len);
void error_print(const char *message);
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val);
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val);
void get_cart_ram_file_name(char *name_buffer);
void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160], const uint_fast8_t line);
void executeRom();
void findFiles();
uint8_t initEmulator();
uint8_t emulation_menu();
void display_pause_overlay();
void draw_emulation_menu(uint8_t selected_tab, uint8_t selected_item, const uint8_t tab_count);
void load_palettes();
bool get_game_palette(uint8_t game_checksum, uint16_t (*game_palette)[4]);
void *memcpy(void *dest, const void *src, size_t count);

InputScancode scancodes[] = 
{
	ScancodeUp,
	ScancodeDown,
	ScancodeLeft, 
	ScancodeRight,
	ScancodePlus,
	ScancodeShift,
	ScancodeClear,
	ScancodeEXE,
	ScancodeKeyboard,
	ScancodeBackspace,
	ScancodeNegative,
};

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;

	/* Pointer to colour palette for each BG, OBJ0, and OBJ1. */
	uint16_t (*selected_palette)[4];
};

struct gb_s gb;
struct priv_t priv =
{
	.rom = NULL,
	.cart_ram = NULL
};

struct palette
{
	/* The name of the palette */
	char name[20];

	/* The actual content of the palette */
	uint16_t data[3][4];
};

const uint16_t default_palette[3][4] = 
{
	{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
	{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
	{ 0x7FFF, 0x5294, 0x294A, 0x0000 }
};

struct palette *color_palettes;

// get the roms in the roms folder
char fileNames[64][100];

uint8_t palette_count = 0;
uint8_t current_palette = 0;
uint8_t current_filename = 0;

int dirFiles = 0;

extern "C"
void main()
{
	/* 
		TODO: 
		 - implement rtc
		 - implement keyinput
	*/
	calcInit(); //backup screen and init some variables

	char *rom_file_name = "\\fls0\\roms\\";

	// make save directory
	mkdir("\\fls0\\gb-saves");

	findFiles();

	// menu
	bool inMenu = true;
	bool buttonPressed = false;

	int8_t menuIndex = 0;

	while(inMenu) 
	{
		// render
		fillScreen(color(0, 0, 0));

		Debug_Printf(0, 25 + menuIndex, true, 0, ">");
		Debug_Printf(0, 24, true, 0, "Detected ROMs (in \\fls0\\roms)");

		for (int i = 0; i < dirFiles; i++)
			Debug_Printf(2, 25 + i, true, 0, "%i. %s", i + 1, fileNames[i]);

		LCD_Refresh();
		
		// wait for keys to be released
		while(Input_IsAnyKeyDown() && buttonPressed) { }
		
		buttonPressed = false;

		if (Input_GetKeyState(&scancodes[CP_KEY_UP])) 
		{
			buttonPressed = true;
			menuIndex--;
			
			if (menuIndex < 0) menuIndex = dirFiles - 1;
		}
		if (Input_GetKeyState(&scancodes[CP_KEY_DOWN])) 
		{
			buttonPressed = true;
			menuIndex++;

			if (menuIndex >= dirFiles) menuIndex = 0;
		}
		if (Input_GetKeyState(&scancodes[CP_KEY_EXE])) 
		{
			buttonPressed = true;
			inMenu = false;

			current_filename = menuIndex;

			strcat(rom_file_name, fileNames[menuIndex]);
		}
	}

	fillScreen(color(0, 0, 0));
	Debug_Printf(0, 0, false, 0, "Loading ROM");
	LCD_Refresh();

	priv.rom = read_rom_to_ram(rom_file_name);

	if(priv.rom == NULL)
	{
		error_print("Error while reading ROM");
		calcEnd();
		return;
	}

	if(!initEmulator())
	{
		calcEnd();
		return;
	}

	executeRom();

	// save cart rom
	char cart_ram_file_name[37];

	get_cart_ram_file_name(cart_ram_file_name);

	// When rom is fully executed, save ram and cleanup
	write_cart_ram_file(cart_ram_file_name, &priv.cart_ram, gb_get_save_size(&gb));

	free(priv.cart_ram);
	free(priv.rom);
	free(color_palettes);

	calcEnd();
}

uint8_t initEmulator()
{
	// LCD_ClearScreen();
	Debug_Printf(0, 0, false, 0, "Init");
	LCD_Refresh();

	enum gb_init_error_e gb_ret;

	/* Initialise emulator context. */
	gb_ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
		&gb_error, &priv);

	switch(gb_ret)
	{
		case GB_INIT_NO_ERROR:
			break;

		case GB_INIT_CARTRIDGE_UNSUPPORTED:
			error_print("Unsupported cartridge");
			return 0;

		case GB_INIT_INVALID_CHECKSUM:
			error_print("ROM checksum failure");
			return 0;

		default:
			error_print("Unknown error on init");
			return 0;
	}

	// load cart rom
	char cart_ram_file_name[37];

	get_cart_ram_file_name(cart_ram_file_name);

	read_cart_ram_file(cart_ram_file_name, &priv.cart_ram, gb_get_save_size(&gb));	

	/* Init gameboy rtc (Just zero everything) */
	struct tm time;
	time.tm_sec = 0;
	time.tm_min = 0;
	time.tm_hour = 0;
	time.tm_yday = 0;

	gb_set_rtc(&gb, &time);

	/* Initialise lcd stuff */
	gb_init_lcd(&gb, &lcd_draw_line);

	load_palettes();

	// load default palette
	priv.selected_palette = color_palettes[0].data;

	// LCD_ClearScreen();
	Debug_Printf(0, 0, false, 0, "Init complete");
	LCD_Refresh();

	return 1;
}

void executeRom() 
{
	uint32_t frame = 1;
	uint8_t draw_frame = 0;

	while (1)
	{
		/* Handle rtc */
		if(frame % 24 == 0)
			gb_tick_rtc(&gb);

		/* Handle Key Input */
		gb.direct.joypad_bits.a = !Input_GetKeyState(&scancodes[CP_KEY_EXE]);
		gb.direct.joypad_bits.b = !Input_GetKeyState(&scancodes[CP_KEY_PLUS]);
		gb.direct.joypad_bits.select = !Input_GetKeyState(&scancodes[CP_KEY_SHIFT]);
		gb.direct.joypad_bits.start = !Input_GetKeyState(&scancodes[CP_KEY_CLEAR]);
		gb.direct.joypad_bits.up = !Input_GetKeyState(&scancodes[CP_KEY_UP]);
		gb.direct.joypad_bits.down = !Input_GetKeyState(&scancodes[CP_KEY_DOWN]);
		gb.direct.joypad_bits.left = !Input_GetKeyState(&scancodes[CP_KEY_LEFT]);
		gb.direct.joypad_bits.right = !Input_GetKeyState(&scancodes[CP_KEY_RIGHT]);

		if(Input_GetKeyState(&scancodes[CP_KEY_KEYBOARD]))
		{
			gb.direct.frame_skip = !gb.direct.frame_skip;

			if(gb.direct.frame_skip)
				error_print("Frameskip on");
			else
				error_print("Frameskip off");
		}

		if(Input_GetKeyState(&scancodes[CP_KEY_BACKSPACE]))
		{
			gb.direct.interlace = !gb.direct.interlace;

			if(gb.direct.interlace)
				error_print("Interlace on");
			else
				error_print("Interlace off");
		}
		

		/* Run CPU until next frame */
		gb_run_frame(&gb);

		/* Check if display should be updated */
		if(gb.direct.frame_skip)
		{
			draw_frame = !draw_frame;

			if(!draw_frame)
				continue;
		}

		if(Input_GetKeyState(&scancodes[CP_KEY_NEGATIVE]))
		{
			if(emulation_menu())
				return;
		}

		/* Update screen with current frame data */
		LCD_Refresh(); 
		frame++;
	}
}

void findFiles() 
{
	char g_path[400] = "\\fls0\\roms\\";
	wchar_t g_wpath[400];

	memset(g_wpath, 0, sizeof(g_wpath));
	
	//convert from char to wchar
	for(int i = 0; g_path[i] != 0; i++)
	{
		wchar_t ch = g_path[i];
		g_wpath[i] = ch;
	}
	
	//add the * to the file path 
	{
		int i = 0;
		while(g_wpath[i] != 0) i++; //seek to the end of the string
		g_wpath[i++]='*'; //add an *
		g_wpath[i  ]= 0 ; //add the 0
	}

	wchar_t fileName[100];
	struct findInfo findInfoBuf;
	int findHandle;
	int ret = findFirst(g_wpath, &findHandle, fileName, &findInfoBuf);

	dirFiles = 0;

	while(ret >= 0) 
	{
		//create dirEntry structure
		memset(&fileNames[dirFiles], 0, sizeof(fileNames[dirFiles]));

		//copy file name
		for(int i = 0; fileName[i] != 0; i++) {
			wchar_t ch = fileName[i];

			fileNames[dirFiles][i] = ch;
		}

		dirFiles++;
		
		//serch the next
		ret = findNext(findHandle, fileName, &findInfoBuf);
	}

	findClose(findHandle);
}

void error_print(const char *message)
{
	Debug_Printf(0, 0, false, 0, message);
	
	for (uint8_t i = 0; i < 100; i++)
		LCD_Refresh();	
}

uint8_t *read_rom_to_ram(const char *file_name)
{
	int32_t rom_file = open(file_name, OPEN_READ);	
	
	if(rom_file < 0)
		return NULL;

	struct stat rom_file_stat;

	fstat(rom_file, &rom_file_stat);

	// dynamically allocate space for rom in heap
	uint8_t *rom = (uint8_t *)malloc(rom_file_stat.fileSize);

	// check if pointer to rom is no nullptr
	if(!rom)
		return NULL;
		
	int32_t status = read(rom_file, rom, rom_file_stat.fileSize);
	
	if(status < 0)
		return NULL;

	close(rom_file);

	return rom;
}

uint8_t emulation_menu()
{
	// show emulation paused text
	display_pause_overlay();

	const uint8_t tab_count = 4;

	bool in_menu = true;
	bool button_pressed = true;

	uint8_t item_counts[tab_count] = { 4, 1, 1, 1 };

	uint8_t selected_tab = 0;
	uint8_t selected_item = 0;


	uint32_t key1;
	uint32_t key2; 

	while(in_menu)
	{
		draw_emulation_menu(selected_tab, selected_item, tab_count);

		while(button_pressed) 
		{ 
			getKey(&key1, &key2);

			if(!(key1 | key2))
				button_pressed = false;
		}
		
		button_pressed = false;

		getKey(&key1, &key2);

		// handle controls
		if(testKey(key1, key2, KEY_NEGATIVE))
		{
			while(Input_IsAnyKeyDown()) { }

			return 0;
		}

		if(testKey(key1, key2, KEY_RIGHT))
		{
			button_pressed = true;

			if(selected_tab != (tab_count - 1))
			{
				selected_tab++;
				selected_item = 0;
			}
		}
		
		if(testKey(key1, key2, KEY_LEFT))
		{
			button_pressed = true;

			if(selected_tab != 0)
			{
				selected_tab--;
				selected_item = 0;
			}
		}
		
		if(testKey(key1, key2, KEY_UP))
		{
			button_pressed = true;

			if(selected_item != 0)
				selected_item--;
		}
		
		if(testKey(key1, key2, KEY_DOWN))
		{
			button_pressed = true;

			if(selected_item != (item_counts[selected_tab] - 1))
				selected_item++;
		}
		
		if(testKey(key1, key2, KEY_EXE))
		{
			button_pressed = true;
			
			// do tab and specific feature
			switch (selected_tab)
			{
			case TAB_INFO:
				switch (selected_item)
				{
				case 0:
					// toggle frameskipping
					gb.direct.frame_skip = !gb.direct.frame_skip;
					break;

				case 1:
					// toggle interlacing
					gb.direct.interlace = !gb.direct.interlace;
					break;

				case 2:
					// switch color palette
					current_palette++;

					if(current_palette >= palette_count)
						current_palette = 0;

					priv.selected_palette = color_palettes[current_palette].data;
					break;

				case 3:
					return 1;
				
				default:
					break;
				}
				break;
			
			default:
				break;
			}
		}
	}

	return 0;
}

void display_pause_overlay()
{
	// go through every pixel of the gameboy screen and darken it
	for(uint16_t y = 0; y < LCD_HEIGHT; y++)
	{
		for(uint16_t x = 0; x < LCD_WIDTH; x++)
		{
			uint32_t pixel = vram[((y * 2) * (LCD_WIDTH * 2)) + (x * 2)];
		
			uint8_t red = ((RGB565_TO_R(pixel) * 13108) / 65536);
			uint8_t green = ((RGB565_TO_G(pixel) * 13108) / 65536);
			uint8_t blue = ((RGB565_TO_B(pixel) * 13108) / 65536);
		
			pixel = RGB_TO_RGB565(red, green, blue); 

			vram[((y * 2) * (LCD_WIDTH * 2)) + (x * 2)] = pixel;
			vram[((y * 2) * (LCD_WIDTH * 2)) + (x * 2) + 1] = pixel;
			vram[(((y * 2) + 1) * (LCD_WIDTH * 2)) + (x * 2)] = pixel;
			vram[(((y * 2) + 1) * (LCD_WIDTH * 2)) + (x * 2) + 1] = pixel;
		}
	}

	// print game paused text
	print_string("Emulation paused", 112, 128, 0, 0xFFFF, 0x0000, 1);
	print_string("Press [(-)] to continue", 91, 148, 0, 0xFFFF, 0x0000, 1);

	LCD_Refresh();
}

void draw_emulation_menu(uint8_t selected_tab, uint8_t selected_item, const uint8_t tab_count)
{
	const uint16_t tab_width = 80;
	const uint16_t tab_height = 18;
	const uint16_t main_y = LCD_HEIGHT * 2;
	const uint16_t main_height = (528 - main_y) - tab_height;
	const uint16_t main_bg = 0x2104;
	const uint16_t bottom_bar_y = 528 - tab_height;
	const uint16_t bottom_bar_bg = 0x39E7;
	const uint16_t bottom_bar_selected = 0x04A0;
	const uint16_t color_success = 0x07E0;
	const uint16_t color_danger = 0xF800;

	// fill menu part of screen
	for(uint16_t y = 0; y < main_height; y++)
	{
		for(uint16_t x = 0; x < (LCD_WIDTH * 2); x++)
			vram[((main_y + y) * (LCD_WIDTH * 2) + x)] = main_bg;
	}
	
	// draw bottom bar
	for(uint16_t y = 0; y < tab_height; y++)
	{
		for(uint16_t x = 0; x < (LCD_WIDTH * 2); x++)
		{
			if(x >= selected_tab * tab_width && x < (selected_tab + 1) * tab_width)
				vram[((bottom_bar_y + y) * (LCD_WIDTH * 2)) + x] = bottom_bar_selected;
			else
				vram[((bottom_bar_y + y) * (LCD_WIDTH * 2)) + x] = bottom_bar_bg;
		}
	}

	for(uint8_t i = 0; i < tab_count; i++)
	{
		// print tab label
		char tab_label[9];

		switch (i)
		{
		case TAB_INFO:
			strcpy(tab_label, "Current");
			break;
		case TAB_LOAD_ROM:
			strcpy(tab_label, "Load");
			break;
		case TAB_SAVESTATES:
			strcpy(tab_label, "Saves");
			break;
		case TAB_SETTINGS:
			strcpy(tab_label, "Settings");
			break;
		
		default:
			break;
		}

		print_string(tab_label, (tab_width * i) + ((tab_width - (strlen(tab_label) * (DEBUG_CHAR_WIDTH - 2))) / 2), 
			512, 0, 0xFFFF, 0x0000, 1);
	}

	// draw menu contents
	switch (selected_tab)
	{
	case TAB_INFO:
		{
			// draw rom title
			for(uint16_t y = 0; y < 37; y++)
			{
				for(uint16_t x = 0; x < (LCD_WIDTH * 2); x++)
					vram[((main_y + y) * (LCD_WIDTH * 2) + x)] = bottom_bar_selected;
			}

			char title_string[200] = " Current ROM: ";
			char tmp_string[100];

			gb_get_rom_name(&gb, tmp_string);
			strcat(title_string, tmp_string);

			print_string(title_string, 0, main_y + 4, 0, 0x0000, 0x0000, 1);

			// draw rom filename
			strcpy(title_string, " Filename: ");
			strcat(title_string, fileNames[current_filename]);

			print_string(title_string, 0, main_y + 20, 0, 0x0000, 0x0000, 1);

			// draw interactive menu
			strcpy(title_string, " Frameskipping                                        ");
			print_string(title_string, 0, main_y + 44, 0, 0xFFFF, (selected_item == 0) * 0x8410, 1);

			strcpy(title_string, (gb.direct.frame_skip)? "Enabled" : "Disabled");
			print_string(title_string, 264 + (gb.direct.frame_skip * 3), main_y + 44, 0, 
				(color_success * gb.direct.frame_skip) + (color_danger * !gb.direct.frame_skip), 0x0000, 1);

			strcpy(title_string, " Interlacing                                          ");
			print_string(title_string, 0, main_y + 58, 0, 0xFFFF, (selected_item == 1) * 0x8410, 1);

			strcpy(title_string, (gb.direct.interlace)? "Enabled" : "Disabled");
			print_string(title_string, 264 + (gb.direct.interlace * 3), main_y + 58, 0, 
				(color_success * gb.direct.interlace) + (color_danger * !gb.direct.interlace), 0x0000, 1);

			strcpy(title_string, " Color Palette                                        ");
			print_string(title_string, 0, main_y + 72, 0, 0xFFFF, (selected_item == 2) * 0x8410, 1);

			print_string(color_palettes[current_palette].name, 312 - (strlen(color_palettes[current_palette].name) * 6),
			 	main_y + 72, 0, color_success, 0x0000, 1);

			strcpy(title_string, " Quit CPBoy                                           ");
			print_string(title_string, 0, main_y + 86, 0, 0xFFFF, (selected_item == 3) * 0x8410, 1);

			break;
		}
	
	default:
		break;
	}
	
	LCD_Refresh();
}

int8_t show_palette_dialog()
{
	LCD_VRAMBackup();

	

	LCD_VRAMRestore();
}

void load_palettes()
{
	char title[25];

	if(palette_count != 0)
		free(color_palettes);

	// check if gamepalette exists
	uint16_t game_palette[3][4];
	
	bool game_palette_exists = get_game_palette(gb_colour_hash(&gb), game_palette);

	palette_count = 1 + game_palette_exists;

	color_palettes = new struct palette[palette_count];

	// Init default palette
	strcpy(color_palettes[0].name, "Default");
	memcpy(color_palettes[0].data, default_palette, sizeof(default_palette));

	// Add game default palette if it exists
	if(!game_palette_exists)
		return;
		
	gb_get_rom_name(&gb, title);
	strcat(title, " Palette");

	strcpy(color_palettes[1].name, title);
	memcpy(color_palettes[1].data, game_palette, sizeof(game_palette));

	// TODO: load custom palettes
}

bool get_game_palette(uint8_t game_checksum, uint16_t (*game_palette)[4])
{
	/* Palettes by deltabeard from the PeanutGB SDL example */
	switch(game_checksum)
	{
		/* Balloon Kid and Tetris Blast */
		case 0x71:
		case 0xFF:
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E60), RGB555_TO_RGB565(0x7C00), 0x0000 }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E60), RGB555_TO_RGB565(0x7C00), 0x0000 }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E60), RGB555_TO_RGB565(0x7C00), 0x0000 }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		/* Pokemon Yellow and Tetris */
		case 0x15:
		case 0xDB:
		case 0x95: /* Not officially */
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7FE0), RGB555_TO_RGB565(0x7C00), 0x0000 }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7FE0), RGB555_TO_RGB565(0x7C00), 0x0000 }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7FE0), RGB555_TO_RGB565(0x7C00), 0x0000 }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		/* Donkey Kong */
		case 0x19:
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E10), RGB555_TO_RGB565(0x48E7), 0x0000 }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E10), RGB555_TO_RGB565(0x48E7), 0x0000 }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E60), RGB555_TO_RGB565(0x7C00), 0x0000 }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		/* Pokemon Blue */
		case 0x61:
		case 0x45:

		/* Pokemon Blue Star */
		case 0xD8:
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E10), RGB555_TO_RGB565(0x48E7), 0x0000 }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x329F), RGB555_TO_RGB565(0x001F), 0x0000 }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x329F), RGB555_TO_RGB565(0x001F), 0x0000 }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		/* Pokemon Red */
		case 0x14:
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x3FE6), RGB555_TO_RGB565(0x0200), 0x0000 }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E10), RGB555_TO_RGB565(0x48E7), 0x0000 }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E10), RGB555_TO_RGB565(0x48E7), 0x0000 }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		/* Pokemon Red Star */
		case 0x8B:
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E10), RGB555_TO_RGB565(0x48E7), 0x0000 }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x329F), RGB555_TO_RGB565(0x001F), 0x0000 }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x3FE6), RGB555_TO_RGB565(0x0200), 0x0000 }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		/* Kirby */
		case 0x27:
		case 0x49:
		case 0x5C:
		case 0xB3:
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7D8A), RGB555_TO_RGB565(0x6800), RGB555_TO_RGB565(0x3000), RGB555_TO_RGB565(0x0000) }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x001F), RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7FEF), RGB555_TO_RGB565(0x021F) }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x527F), RGB555_TO_RGB565(0x7FE0), RGB555_TO_RGB565(0x0180), RGB555_TO_RGB565(0x0000) }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		/* Donkey Kong Land [1/2/III] */
		case 0x18:
		case 0x6A:
		case 0x4B:
		case 0x6B:
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7F08), RGB555_TO_RGB565(0x7F40), RGB555_TO_RGB565(0x48E0), RGB555_TO_RGB565(0x2400) }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x2EFF), RGB555_TO_RGB565(0x7C00), RGB555_TO_RGB565(0x001F) }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x463B), RGB555_TO_RGB565(0x2951), RGB555_TO_RGB565(0x0000) }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		/* Link's Awakening */
		case 0x70:
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x03E0), RGB555_TO_RGB565(0x1A00), RGB555_TO_RGB565(0x0120) }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x329F), RGB555_TO_RGB565(0x001F), RGB555_TO_RGB565(0x001F) }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7E10), RGB555_TO_RGB565(0x48E7), RGB555_TO_RGB565(0x0000) }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		/* Mega Man [1/2/3] & others I don't care about. */
		case 0x01:
		case 0x10:
		case 0x29:
		case 0x52:
		case 0x5D:
		case 0x68:
		case 0x6D:
		case 0xF6:
		{
			const uint16_t palette[3][4] =
			{
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x329F), RGB555_TO_RGB565(0x001F), 0x0000 }, /* OBJ0 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x3FE6), RGB555_TO_RGB565(0x0200), 0x0000 }, /* OBJ1 */
				{ RGB555_TO_RGB565(0x7FFF), RGB555_TO_RGB565(0x7EAC), RGB555_TO_RGB565(0x40C0), 0x0000 }  /* BG */
			};
			memcpy(game_palette, palette, sizeof(palette));
			return 1;
		}

		default:
			return 0;
	}
}

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = (priv_t *)gb->direct.priv;
	return p->rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = (priv_t *)gb->direct.priv;
	return p->cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, 
	const uint8_t val)
{
	const struct priv_t * const p = (priv_t*)gb->direct.priv;
	p->cart_ram[addr] = val;
}

/**
 * Reads the cart ram (savegame) from a file. Returns 0 if everything 
 * went well, else something went wrong. 
 */
uint8_t read_cart_ram_file(const char *save_file_name, uint8_t **dest,
			const size_t len)
{
	int f;

	/* If save file not required. */
	if(len == 0)
	{
		*dest = NULL;
		return 0;
	}

	/* Allocate enough memory to hold save file. */
	*dest = (uint8_t *)malloc(len);

	if(!*dest)
		return 1;

	f = open(save_file_name, OPEN_READ);

	/* It doesn't matter if the save file doesn't exist. We initialise the
	 * save memory allocated above. The save file will be created on exit. */
	if(f < 0)
	{
		for(uint32_t x = 0; x < len; x++)
			(*dest)[x] = 0;	

		return 0;
	}

	/* Read save file to allocated memory. */
	read(f, *dest, len);
	close(f);

	return 0;
}

/**
 * Writes the cart ram (savegame) to a file. Returns 0 if everything 
 * went well, else something went wrong. 
 */
uint8_t write_cart_ram_file(const char *save_file_name, uint8_t **dest,
			const size_t len)
{
	int f;

	if(len == 0 || !*dest)
		return 0;

	f = open(save_file_name, OPEN_CREATE | OPEN_WRITE);

	if(f < 0)
		return 1;

	/* Record save file. */
	write(f, *dest, len);
	close(f);

	return 0;
}

/*  
 * Gets the filename of the current roms cart ram save
 * Make sure the buffer is big enough
 */
void get_cart_ram_file_name(char *name_buffer)
{
	strcpy(name_buffer, "\\fls0\\gb-saves\\");

	char temp[17];

	gb_get_rom_name(&gb, temp);

	strcat(name_buffer, temp);	
	strcat(name_buffer, ".csav");
}

/**
 * Handles an error reported by the emulator. The emulator context may be used
 * to better understand why the error given in gb_err was reported.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val)
{
	switch(gb_err)
	{
		case GB_INVALID_OPCODE:
			Debug_Printf(0, 0, false, 0, "Invalid opcode %#04x at PC: %#06x, SP: %#06x\n",
				val,
				gb->cpu_reg.pc - 1,
				gb->cpu_reg.sp);
	
			for (uint8_t i = 0; i < 100; i++)
				LCD_Refresh();	
			break;

		/* Ignoring non fatal errors. */
		case GB_INVALID_WRITE:
		case GB_INVALID_READ:
			return;

		default:
			error_print("Unknown error while executing");
			break;
	}

	return;
}

/**
 * Draws scanline into framebuffer.
 */
void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160],
	const uint_fast8_t line)
{
	struct priv_t *priv = (priv_t*)gb->direct.priv;
	
	for(uint16_t x = 0; x < LCD_WIDTH; x++)
	{
		uint16_t color = priv->selected_palette
						[(pixels[x] & LCD_PALETTE_ALL) >> 4]
				    [pixels[x] & 3];

		vram[(line * (LCD_WIDTH * 4)) + (x * 2)] = color;
		vram[(line * (LCD_WIDTH * 4)) + (x * 2) + 1] = color;
		vram[(line * (LCD_WIDTH * 4)) + (LCD_WIDTH * 2) + (x * 2)] = color;
		vram[(line * (LCD_WIDTH * 4)) + (LCD_WIDTH * 2) + (x * 2) + 1] = color;
	}
}

void *memcpy(void *dest, const void *src, size_t count)
{
	for (size_t i = 0; i < count; i = i + 1)
		((uint8_t *) dest)[i] = ((uint8_t *) src)[i];

	return (dest);
}
