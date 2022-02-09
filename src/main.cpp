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

#define GB_KEY_UP				0
#define GB_KEY_DOWN			1
#define GB_KEY_LEFT			2
#define GB_KEY_RIGHT		3
#define GB_KEY_A				4
#define GB_KEY_B				5
#define GB_KEY_START		6
#define GB_KEY_SELECT		7

#define CP_KEY_UP					0
#define CP_KEY_DOWN				1
#define CP_KEY_LEFT				2
#define CP_KEY_RIGHT			3
#define CP_KEY_PLUS				4
#define CP_KEY_SHIFT			5
#define CP_KEY_CLEAR			6
#define CP_KEY_EXE				7
#define CP_KEY_KEYBOARD		8		
#define CP_KEY_BACKSPACE	9			
#define CP_KEY_NEGATIVE		10		

#define CONTROLS_COUNT	(sizeof(controls) / 8)

#define TAB_INFO 				0
#define TAB_SAVESTATES 	1
#define TAB_LOAD_ROM 		2
#define TAB_SETTINGS 		3

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
void show_palette_dialog();
void show_edit_palette_dialog(struct palette *palette);
void show_controls_dialog();
void show_select_key_dialog(uint32_t *key_controls);
void draw_color_edit_panel(uint16_t x, uint16_t y, uint16_t width, uint8_t selected_color_rect, 
	char *title, uint16_t *colors, int8_t selected_item);
void draw_slider(uint16_t x, uint16_t y, uint16_t width, uint16_t track_color,
	uint16_t handle_color, uint16_t max_value, uint16_t value);
void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
	uint16_t color, uint16_t border, uint16_t border_color);
void load_palettes();
void create_palette(char *palette_name);
void convert_byte_to_string(uint8_t byte, char *string);
uint8_t convert_string_to_byte(char *string); 
int8_t save_palette(struct palette *palette);
void delete_palette(struct palette *palette);
int8_t save_controls(uint32_t (*controls_ptr)[2]);
void load_controls(uint32_t (*controls_ptr)[2]);
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
	char name[25];

	/* The actual content of the palette */
	uint16_t data[3][4];
};

uint16_t default_palette[3][4];

struct palette *color_palettes = NULL;

uint32_t controls[8][2];

// get the roms in the roms folder
char fileNames[64][100];

uint8_t palette_count;
uint8_t current_palette;
uint8_t current_filename;

bool game_palette_exists;
bool controls_changed;

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

	// init palette stuff
	{
		color_palettes = NULL;
		current_palette = 0;

		const uint16_t _default_palette[3][4] = 
		{
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 }
		};
		
		memcpy(default_palette, _default_palette, sizeof(default_palette));
	}

	controls_changed = false;

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

	// save user stuff
	if(controls_changed)
		save_controls(controls);

	calcEnd();
}

uint8_t initEmulator()
{
	// LCD_ClearScreen();
	Debug_Printf(0, 0, false, 0, "Init");
	LCD_Refresh();

	// Load user configs
	load_controls(controls);

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

	uint8_t item_counts[tab_count] = { 4, 1, 1, 2 };

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
			case TAB_SETTINGS:
				switch (selected_item)
				{
				case 0:
					show_palette_dialog();
					break;
				case 1:
					show_controls_dialog();
					break;				
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
	case TAB_SETTINGS:
		{
			char title_string[200];

			// draw settings title
			for(uint16_t y = 0; y < 37; y++)
			{
				for(uint16_t x = 0; x < (LCD_WIDTH * 2); x++)
					vram[((main_y + y) * (LCD_WIDTH * 2) + x)] = bottom_bar_selected;
			}

			print_string(" Global Settings", 0, main_y + 12, 0, 0x0000, 0x0000, 1);

			// draw interactive menu
			strcpy(title_string, " Custom Color Palettes                                ");
			print_string(title_string, 0, main_y + 44, 0, 0xFFFF, (selected_item == 0) * 0x8410, 1);

			strcpy(title_string, " Controls                                             ");
			print_string(title_string, 0, main_y + 58, 0, 0xFFFF, (selected_item == 1) * 0x8410, 1);

			break;
		}
	default:
		break;
	}
	
	LCD_Refresh();
}

void show_palette_dialog()
{
	uint16_t lcd_bak[528][320];

	// backup lcd
	for(uint16_t y = 0; y < 528; y++)
	{
		for(uint16_t x = 0; x < 320; x++)
			lcd_bak[y][x] = vram[(y * 320) + x];
	}
	
	const uint16_t subtitle_fg = 0xB5B6;
	const uint16_t dialog_width = 200;
	const uint16_t dialog_border = 0x04A0;
	const uint16_t dialog_bg = 0x2104;

	uint8_t selected_item = 0;

	uint32_t key1;
	uint32_t key2;

	bool button_pressed = true;
	bool in_menu = true;

	while (in_menu)
	{
		const uint8_t custom_palette_count = palette_count - 1 - game_palette_exists;

		const uint16_t dialog_height = 112 + (((custom_palette_count)? custom_palette_count : 1) * 14);
		const uint16_t dialog_y = (528 - dialog_height) / 2;
		const uint16_t dialog_x = (320 - dialog_width) / 2;

		// draw dialog background
		for (uint16_t y = 0; y < dialog_height; y++)
		{
			for (uint16_t x = 0; x < dialog_width; x++)
			{
				if(y == 0 || y == (dialog_height - 1) || x == 0 || x == (dialog_width - 1))
					vram[((y + dialog_y) * (LCD_WIDTH * 2)) + x + dialog_x] = dialog_border;
				else
					vram[((y + dialog_y) * (LCD_WIDTH * 2)) + x + dialog_x] = dialog_bg;
			}
		}

		// draw title
		print_string("Custom Color Palettes", 96, dialog_y + 5, 0, dialog_border, 0x0000, 1);

		// draw subtitle
		print_string("[EXE] to edit", 120, dialog_y + 23, 0, subtitle_fg, 0x0000, 1);
		print_string("[CLEAR] to remove", 108, dialog_y + 37, 0, subtitle_fg, 0x0000, 1);

		// draw all palettes
		uint8_t custom_offset = 1 + game_palette_exists;

		for (uint8_t i = 0; i < custom_palette_count; i++)
		{
			print_string(color_palettes[custom_offset + i].name, (320 - (strlen(color_palettes[custom_offset + i].name) * 6)) / 2, 
				dialog_y + 65 + (i * 14), 0, 0xFFFF, (selected_item == i) * 0x8410, 1);
		}		

		// draw no palettes alert if there are no palettes
		if(!custom_palette_count)
			print_string("No custom palettes", 105, dialog_y + 65, 0, dialog_border, 0x0000, 1);

		// draw action buttons
		print_string(" Create Palette ", 111, dialog_y + (((custom_palette_count)? custom_palette_count : 1) * 14) + 79, 0, 
			0xFFFF, (selected_item == custom_palette_count) * 0x8410, 1);
		print_string("      Done      ", 111, dialog_y + (((custom_palette_count)? custom_palette_count : 1) * 14) + 93, 0, 
			0xFFFF, (selected_item == (custom_palette_count + 1)) * 0x8410, 1);

		LCD_Refresh();

		// Wait for release
		while(button_pressed) 
		{ 
			getKey(&key1, &key2);

			if(!(key1 | key2))
				button_pressed = false;
		}

		getKey(&key1, &key2);

		// handle controls		
		if(testKey(key1, key2, KEY_UP))
		{
			button_pressed = true;

			if(selected_item != 0)
				selected_item--;
		}
		
		if(testKey(key1, key2, KEY_DOWN))
		{
			button_pressed = true;

			if(selected_item != (custom_palette_count + 1))
				selected_item++;
		}
		
		if(testKey(key1, key2, KEY_EXE))
		{
			button_pressed = true;

			if(selected_item == (custom_palette_count + 1))
			{
				in_menu = false; // close color palette menu
			}
			else if(selected_item == (custom_palette_count))
			{
				// create new palette
				char palette_name[100] = "Custom ";
				char number[4];

				uint8_t lowest = 0;

				// get current lowest number
				for(uint8_t i = 0; i < custom_palette_count; i++)
				{
					uint8_t current_number = convert_string_to_byte(color_palettes[i + custom_offset].name + sizeof("Custom ") - 1);

					// Debug_Printf(0, 8, 0, 0, color_palettes[i + custom_offset].name + sizeof("Custom ") - 1);
					// Debug_Printf(0, 9, 0, 0, color_palettes[i + custom_offset].name);

					if(lowest == current_number)
					{
						lowest++;
						i = 255; 	// restart for loop
					}

					// Debug_Printf(0, 10, 0, 0, "Current: %d; Lowest: %d; i: %d", current_number, lowest, i);

					// for (uint8_t i = 0; i < 60; i++)
					// 	LCD_Refresh();
				}

				convert_byte_to_string(lowest, number);
				strcat(palette_name, number);
				
				create_palette(palette_name);
				load_palettes();
			}
			else
			{
				// edit palette
				show_edit_palette_dialog(&color_palettes[custom_offset + selected_item]);
				save_palette(&color_palettes[custom_offset + selected_item]);

				// restore lcd
				for(uint16_t y = 0; y < 528; y++)
				{
					for(uint16_t x = 0; x < 320; x++)
						vram[(y * 320) + x] = lcd_bak[y][x];
				}
			}
		}

		if(testKey(key1, key2, KEY_CLEAR))
		{
			button_pressed = true;

			// delete palette
			delete_palette(&color_palettes[custom_offset + selected_item]);
			load_palettes();

			// restore lcd
			for(uint16_t y = 0; y < 528; y++)
			{
				for(uint16_t x = 0; x < 320; x++)
					vram[(y * 320) + x] = lcd_bak[y][x];
			}
		}
	}

	// restore lcd
	for(uint16_t y = 0; y < 528; y++)
	{
		for(uint16_t x = 0; x < 320; x++)
			vram[(y * 320) + x] = lcd_bak[y][x];
	}
}

void show_edit_palette_dialog(struct palette *palette_to_edit)
{
	const uint16_t dialog_width = 204;
	const uint16_t dialog_height = 347;
	const uint16_t dialog_border = 0x04A0;
	const uint16_t dialog_bg = 0x2104;
	const uint16_t dialog_y = (528 - dialog_height) / 2;
	const uint16_t dialog_x = (320 - dialog_width) / 2;

	uint32_t key1;
	uint32_t key2;

	uint8_t selected_item = 0;
	uint8_t selected_color_rect_obj0 = 0;
	uint8_t selected_color_rect_obj1 = 0;
	uint8_t selected_color_rect_bg = 0;
	uint8_t hold_amount = 0;

	bool in_menu = true;
	bool button_pressed = true;
	bool holding_button = false;

	while (in_menu)
	{
		// draw dialog background
		for (uint16_t y = 0; y < dialog_height; y++)
		{
			for (uint16_t x = 0; x < dialog_width; x++)
			{
				if(y == 0 || y == (dialog_height - 1) || x == 0 || x == (dialog_width - 1))
					vram[((y + dialog_y) * (LCD_WIDTH * 2)) + x + dialog_x] = dialog_border;
				else
					vram[((y + dialog_y) * (LCD_WIDTH * 2)) + x + dialog_x] = dialog_bg;
			}
		}

		// draw title
		char title[100] = "Edit ";
		strcat(title, palette_to_edit->name);

		print_string(title, (320 - (strlen(title) * 6)) / 2, dialog_y + 5, 0, dialog_border, 0x0000, 1);

		// draw color edit panels
		draw_color_edit_panel(dialog_x + 2, dialog_y + 33, dialog_width - 4, selected_color_rect_obj0, "OBJ0", 
			palette_to_edit->data[0], selected_item);

		draw_color_edit_panel(dialog_x + 2, dialog_y + 130, dialog_width - 4, selected_color_rect_obj1, "OBJ1", 
			palette_to_edit->data[1], selected_item - 4);		

		draw_color_edit_panel(dialog_x + 2, dialog_y + 227, dialog_width - 4, selected_color_rect_bg, "Background", 
			palette_to_edit->data[2], selected_item - 8);		

		// draw done button
		print_string("      Done      ", 111, dialog_y + 328, 0, 0xFFFF, 0x8410 * (selected_item == 12), 1); 

		LCD_Refresh();

		// Wait for release
		while(button_pressed) 
		{ 
			getKey(&key1, &key2);

			if(!(key1 | key2))
			{
				button_pressed = false;
				holding_button = false;
			}

			LCD_Refresh(); // LCD_Refresh to create a delay

			if(hold_amount++ > 20 || (holding_button && hold_amount > 3))
			{
				holding_button = true;
				button_pressed = false;
			}
		}

		hold_amount = 0;

		getKey(&key1, &key2);

		// handle controls		
		if(testKey(key1, key2, KEY_UP))
		{
			button_pressed = true;

			if(selected_item != 0)
				selected_item--;
		}

		if(testKey(key1, key2, KEY_DOWN))
		{
			button_pressed = true;

			if(selected_item != 12)
				selected_item++;
		}

		if(testKey(key1, key2, KEY_LEFT))
		{
			button_pressed = true;

			switch (selected_item)
			{
			// color rectangle selection
			case 0:
				if(selected_color_rect_obj0 != 0)
					selected_color_rect_obj0--;
				break;
			case 4:
				if(selected_color_rect_obj1 != 0)
					selected_color_rect_obj1--;
				break;
			case 8:
				if(selected_color_rect_bg != 0)
					selected_color_rect_bg--;
				break;

			// slider interaction
			case 1:
				if(RGB565_TO_R(palette_to_edit->data[0][selected_color_rect_obj0]) != 0)
					palette_to_edit->data[0][selected_color_rect_obj0] = RGB_TO_RGB565(
						(RGB565_TO_R(palette_to_edit->data[0][selected_color_rect_obj0]) - 1), 
						RGB565_TO_G(palette_to_edit->data[0][selected_color_rect_obj0]),
						RGB565_TO_B(palette_to_edit->data[0][selected_color_rect_obj0]));
				break;
			case 2:
				if(RGB565_TO_G(palette_to_edit->data[0][selected_color_rect_obj0]) != 0)
					palette_to_edit->data[0][selected_color_rect_obj0] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[0][selected_color_rect_obj0]), 
						(RGB565_TO_G(palette_to_edit->data[0][selected_color_rect_obj0]) - 1),
						RGB565_TO_B(palette_to_edit->data[0][selected_color_rect_obj0]));
				break;
			case 3:
				if(RGB565_TO_B(palette_to_edit->data[0][selected_color_rect_obj0]) != 0)
					palette_to_edit->data[0][selected_color_rect_obj0] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[0][selected_color_rect_obj0]), 
						RGB565_TO_G(palette_to_edit->data[0][selected_color_rect_obj0]),
						(RGB565_TO_B(palette_to_edit->data[0][selected_color_rect_obj0]) - 1));
				break;
			case 5:
				if(RGB565_TO_R(palette_to_edit->data[1][selected_color_rect_obj1]) != 0)
					palette_to_edit->data[1][selected_color_rect_obj1] = RGB_TO_RGB565(
						(RGB565_TO_R(palette_to_edit->data[1][selected_color_rect_obj1]) - 1), 
						RGB565_TO_G(palette_to_edit->data[1][selected_color_rect_obj1]),
						RGB565_TO_B(palette_to_edit->data[1][selected_color_rect_obj1]));
				break;
			case 6:
				if(RGB565_TO_G(palette_to_edit->data[1][selected_color_rect_obj1]) != 0)
					palette_to_edit->data[1][selected_color_rect_obj1] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[1][selected_color_rect_obj1]), 
						(RGB565_TO_G(palette_to_edit->data[1][selected_color_rect_obj1]) - 1),
						RGB565_TO_B(palette_to_edit->data[1][selected_color_rect_obj1]));
				break;
			case 7:
				if(RGB565_TO_B(palette_to_edit->data[1][selected_color_rect_obj1]) != 0)
					palette_to_edit->data[1][selected_color_rect_obj1] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[1][selected_color_rect_obj1]), 
						RGB565_TO_G(palette_to_edit->data[1][selected_color_rect_obj1]),
						(RGB565_TO_B(palette_to_edit->data[1][selected_color_rect_obj1]) - 1));
				break;
			case 9:
				if(RGB565_TO_R(palette_to_edit->data[2][selected_color_rect_bg]) != 0)
					palette_to_edit->data[2][selected_color_rect_bg] = RGB_TO_RGB565(
						(RGB565_TO_R(palette_to_edit->data[2][selected_color_rect_bg]) - 1), 
						RGB565_TO_G(palette_to_edit->data[2][selected_color_rect_bg]),
						RGB565_TO_B(palette_to_edit->data[2][selected_color_rect_bg]));
				break;
			case 10:
				if(RGB565_TO_G(palette_to_edit->data[2][selected_color_rect_bg]) != 0)
					palette_to_edit->data[2][selected_color_rect_bg] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[2][selected_color_rect_bg]), 
						(RGB565_TO_G(palette_to_edit->data[2][selected_color_rect_bg]) - 1),
						RGB565_TO_B(palette_to_edit->data[2][selected_color_rect_bg]));
				break;
			case 11:
				if(RGB565_TO_B(palette_to_edit->data[2][selected_color_rect_bg]) != 0)
					palette_to_edit->data[2][selected_color_rect_bg] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[2][selected_color_rect_bg]), 
						RGB565_TO_G(palette_to_edit->data[2][selected_color_rect_bg]),
						(RGB565_TO_B(palette_to_edit->data[2][selected_color_rect_bg]) - 1));
				break;
			default:
				break;
			}
		}

		if(testKey(key1, key2, KEY_RIGHT))
		{
			button_pressed = true;

			switch (selected_item)
			{
			// color rectangle selection
			case 0:
				if(selected_color_rect_obj0 != 3)
					selected_color_rect_obj0++;
				break;
			case 4:
				if(selected_color_rect_obj1 != 3)
					selected_color_rect_obj1++;
				break;
			case 8:
				if(selected_color_rect_bg != 3)
					selected_color_rect_bg++;
				break;

			// slider interaction
			case 1:
				if(RGB565_TO_R(palette_to_edit->data[0][selected_color_rect_obj0]) != 0b11111)
					palette_to_edit->data[0][selected_color_rect_obj0] = RGB_TO_RGB565(
						(RGB565_TO_R(palette_to_edit->data[0][selected_color_rect_obj0]) + 1), 
						RGB565_TO_G(palette_to_edit->data[0][selected_color_rect_obj0]),
						RGB565_TO_B(palette_to_edit->data[0][selected_color_rect_obj0]));
				break;
			case 2:
				if(RGB565_TO_G(palette_to_edit->data[0][selected_color_rect_obj0]) != 0b111111)
					palette_to_edit->data[0][selected_color_rect_obj0] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[0][selected_color_rect_obj0]), 
						(RGB565_TO_G(palette_to_edit->data[0][selected_color_rect_obj0]) + 1),
						RGB565_TO_B(palette_to_edit->data[0][selected_color_rect_obj0]));
				break;
			case 3:
				if(RGB565_TO_B(palette_to_edit->data[0][selected_color_rect_obj0]) != 0b11111)
					palette_to_edit->data[0][selected_color_rect_obj0] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[0][selected_color_rect_obj0]), 
						RGB565_TO_G(palette_to_edit->data[0][selected_color_rect_obj0]),
						(RGB565_TO_B(palette_to_edit->data[0][selected_color_rect_obj0]) + 1));
				break;
			case 5:
				if(RGB565_TO_R(palette_to_edit->data[1][selected_color_rect_obj1]) != 0b11111)
					palette_to_edit->data[1][selected_color_rect_obj1] = RGB_TO_RGB565(
						(RGB565_TO_R(palette_to_edit->data[1][selected_color_rect_obj1]) + 1), 
						RGB565_TO_G(palette_to_edit->data[1][selected_color_rect_obj1]),
						RGB565_TO_B(palette_to_edit->data[1][selected_color_rect_obj1]));
				break;
			case 6:
				if(RGB565_TO_G(palette_to_edit->data[1][selected_color_rect_obj1]) != 0b111111)
					palette_to_edit->data[1][selected_color_rect_obj1] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[1][selected_color_rect_obj1]), 
						(RGB565_TO_G(palette_to_edit->data[1][selected_color_rect_obj1]) + 1),
						RGB565_TO_B(palette_to_edit->data[1][selected_color_rect_obj1]));
				break;
			case 7:
				if(RGB565_TO_B(palette_to_edit->data[1][selected_color_rect_obj1]) != 0b11111)
					palette_to_edit->data[1][selected_color_rect_obj1] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[1][selected_color_rect_obj1]), 
						RGB565_TO_G(palette_to_edit->data[1][selected_color_rect_obj1]),
						(RGB565_TO_B(palette_to_edit->data[1][selected_color_rect_obj1]) + 1));
				break;
			case 9:
				if(RGB565_TO_R(palette_to_edit->data[2][selected_color_rect_bg]) != 0b11111)
					palette_to_edit->data[2][selected_color_rect_bg] = RGB_TO_RGB565(
						(RGB565_TO_R(palette_to_edit->data[2][selected_color_rect_bg]) + 1), 
						RGB565_TO_G(palette_to_edit->data[2][selected_color_rect_bg]),
						RGB565_TO_B(palette_to_edit->data[2][selected_color_rect_bg]));
				break;
			case 10:
				if(RGB565_TO_G(palette_to_edit->data[2][selected_color_rect_bg]) != 0b111111)
					palette_to_edit->data[2][selected_color_rect_bg] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[2][selected_color_rect_bg]), 
						(RGB565_TO_G(palette_to_edit->data[2][selected_color_rect_bg]) + 1),
						RGB565_TO_B(palette_to_edit->data[2][selected_color_rect_bg]));
				break;
			case 11:
				if(RGB565_TO_B(palette_to_edit->data[2][selected_color_rect_bg]) != 0b11111)
					palette_to_edit->data[2][selected_color_rect_bg] = RGB_TO_RGB565(
						RGB565_TO_R(palette_to_edit->data[2][selected_color_rect_bg]), 
						RGB565_TO_G(palette_to_edit->data[2][selected_color_rect_bg]),
						(RGB565_TO_B(palette_to_edit->data[2][selected_color_rect_bg]) + 1));
				break;
			default:
				break;
			}
		}

		if(testKey(key1, key2, KEY_EXE))
		{
			if(selected_item == 12)
			{
				button_pressed = true;
				in_menu = false;
			}
		}
	}
}

void draw_color_edit_panel(uint16_t x, uint16_t y, uint16_t width, uint8_t selected_color_rect, 
	char *title, uint16_t *colors, int8_t selected_item)
{
	const uint16_t dialog_bg = 0x2104;
	const uint16_t color_rect_width = 47;
	const uint16_t color_rect_height = 25;
	const uint16_t item_selected_color = 0x04A0;
	const uint16_t slider_track_color = 0xB5B6;
	const uint16_t slider_offset = 42;

	// draw title
	print_string(title, x + 5, y, 0, 0xFFFF, 0x0000, 1);

	// draw obj0 color selection
	draw_rectangle(x + 4, y + 15, width - 8, color_rect_height + 4, dialog_bg,
		(selected_item == 0) * 2, item_selected_color);

	for(uint8_t i = 0; i < 4; i++)
	{
		draw_rectangle(x + 6 + (i * color_rect_width), y + 17, color_rect_width,
			color_rect_height, colors[i], 2 * (selected_color_rect == i), 
			item_selected_color);
	}

	// draw obj0 color sliders and description
	print_string("Red", x + 5, y + 46, 0, 0xFFFF, 0x0000, 1);
	draw_slider(x + slider_offset, y + 47, width - slider_offset - 7, slider_track_color, 
	((selected_item == 1) * item_selected_color) + (!(selected_item == 1) * 0xFFFF), 
		0b11111, RGB565_TO_R(colors[selected_color_rect]));

	print_string("Green", x + 5, y + 60, 0, 0xFFFF, 0x0000, 1);
	draw_slider(x + slider_offset, y + 61, width - slider_offset - 7, slider_track_color, 
	((selected_item == 2) * item_selected_color) + (!(selected_item == 2) * 0xFFFF), 
		0b111111, RGB565_TO_G(colors[selected_color_rect]));

	print_string("Blue", x + 5, y + 74, 0, 0xFFFF, 0x0000, 1);
	draw_slider(x + slider_offset, y + 75, width - slider_offset - 7, slider_track_color, 
	((selected_item == 3) * item_selected_color) + (!(selected_item == 3) * 0xFFFF), 
		0b11111, RGB565_TO_B(colors[selected_color_rect]));
}

void show_controls_dialog()
{
	uint16_t lcd_bak[528][320];

	// backup lcd
	for(uint16_t y = 0; y < 528; y++)
	{
		for(uint16_t x = 0; x < 320; x++)
			lcd_bak[y][x] = vram[(y * 320) + x];
	}
	
	const uint16_t subtitle_fg = 0xB5B6;
	const uint16_t dialog_width = 150;
	const uint16_t dialog_height = 80 + (CONTROLS_COUNT * 14);
	const uint16_t dialog_y = (528 - dialog_height) / 2;
	const uint16_t dialog_x = (320 - dialog_width) / 2;
	const uint16_t dialog_border = 0x04A0;
	const uint16_t dialog_bg = 0x2104;

	uint8_t selected_item = 0;

	uint32_t key1;
	uint32_t key2;

	bool button_pressed = true;
	bool in_menu = true;

	while (in_menu)
	{
		// draw dialog background
		for (uint16_t y = 0; y < dialog_height; y++)
		{
			for (uint16_t x = 0; x < dialog_width; x++)
			{
				if(y == 0 || y == (dialog_height - 1) || x == 0 || x == (dialog_width - 1))
					vram[((y + dialog_y) * (LCD_WIDTH * 2)) + x + dialog_x] = dialog_border;
				else
					vram[((y + dialog_y) * (LCD_WIDTH * 2)) + x + dialog_x] = dialog_bg;
			}
		}

		// draw title
		print_string("Controls", 135, dialog_y + 5, 0, dialog_border, 0x0000, 1);

		// draw subtitle
		print_string("[EXE] to edit", 120, dialog_y + 23, 0, subtitle_fg, 0x0000, 1);

		// draw all controls
		for (uint8_t i = 0; i < CONTROLS_COUNT; i++)
		{
			char title[11];

			switch (i)
			{
			case GB_KEY_UP:
				strcpy(title, "UP");
				break;
			case GB_KEY_DOWN:
				strcpy(title, "DOWN");
				break;
			case GB_KEY_LEFT:
				strcpy(title, "LEFT");
				break;
			case GB_KEY_RIGHT:
				strcpy(title, "RIGHT");
				break;
			case GB_KEY_A:
				strcpy(title, "A");
				break;
			case GB_KEY_B:
				strcpy(title, "B");
				break;
			case GB_KEY_START:
				strcpy(title, "START");
				break;
			case GB_KEY_SELECT:
				strcpy(title, "SELECT");
				break;
			
			default:
				break;
			}

			print_string(title, dialog_x + 8, dialog_y + 51 + (i * 14), 0, 0xFFFF, 0x0000, 1);

			switch (controls[i][0])
			{
			case KEY_SHIFT:
				strcpy(title, "[SHIFT]");
				break;
			case KEY_CLEAR:
				strcpy(title, "[CLEAR]");
				break;
			case KEY_BACKSPACE:
				strcpy(title, "[<--]");
				break;
			case KEY_LEFT:
				strcpy(title, "[LEFT]");
				break;
			case KEY_RIGHT:
				strcpy(title, "[RIGHT]");
				break;
			case KEY_Z:
				strcpy(title, "[Z]");
				break;
			case KEY_POWER:
				strcpy(title, "[^]");
				break;
			case KEY_DIVIDE:
				strcpy(title, "[/]");
				break;
			case KEY_MULTIPLY:
				strcpy(title, "[*]");
				break;
			case KEY_SUBTRACT:
				strcpy(title, "[-]");
				break;
			case KEY_ADD:
				strcpy(title, "[+]");
				break;
			case KEY_EXE:
				strcpy(title, "[EXE]");
				break;
			case KEY_EXP:
				strcpy(title, "[EXP]");
				break;
			case KEY_3:
				strcpy(title, "[3]");
				break;
			case KEY_6:
				strcpy(title, "[6]");
				break;
			case KEY_9:
				strcpy(title, "[9]");
				break;
			
			default:
				break;
			}

			switch (controls[i][1])
			{
			case KEY_KEYBOARD:
				strcpy(title, "[KEYBOARD]");
				break;
			case KEY_UP:
				strcpy(title, "[UP]");
				break;
			case KEY_DOWN:
				strcpy(title, "[DOWN]");
				break;
			case KEY_EQUALS:
				strcpy(title, "[=]");
				break;
			case KEY_X:
				strcpy(title, "[X]");
				break;
			case KEY_Y:
				strcpy(title, "[Y]");
				break;
			case KEY_LEFT_BRACKET:
				strcpy(title, "[(]");
				break;
			case KEY_RIGHT_BRACKET:
				strcpy(title, "[)]");
				break;
			case KEY_COMMA:
				strcpy(title, "[,]");
				break;
			case KEY_NEGATIVE:
				strcpy(title, "[(-)]");
				break;
			case KEY_0:
				strcpy(title, "[0]");
				break;
			case KEY_DOT:
				strcpy(title, "[.]");
				break;
			case KEY_1:
				strcpy(title, "[1]");
				break;
			case KEY_2:
				strcpy(title, "[2]");
				break;
			case KEY_4:
				strcpy(title, "[4]");
				break;
			case KEY_5:
				strcpy(title, "[5]");
				break;
			case KEY_7:
				strcpy(title, "[7]");
				break;
			case KEY_8:
				strcpy(title, "[8]");
				break;
			
			default:
				break;
			}
		
			if(!(controls[i][0] | controls[i][1]))
				strcpy(title, "NONE");

			print_string(title, dialog_x + dialog_width - 10 - (strlen(title) * 6), dialog_y + 51 + (i * 14), 0, 0xFFFF, 
				(selected_item == i) * 0x8410, 1);
		}
			
		// draw action buttons
		print_string("      Done      ", 111, dialog_y + (CONTROLS_COUNT * 14) + 61, 0, 
			0xFFFF, (selected_item == CONTROLS_COUNT) * 0x8410, 1);

		LCD_Refresh();

		// Wait for release
		while(button_pressed) 
		{ 
			getKey(&key1, &key2);

			if(!(key1 | key2))
				button_pressed = false;
		}

		getKey(&key1, &key2);

		// handle controls		
		if(testKey(key1, key2, KEY_UP))
		{
			button_pressed = true;

			if(selected_item != 0)
				selected_item--;
		}
		
		if(testKey(key1, key2, KEY_DOWN))
		{
			button_pressed = true;

			if(selected_item != CONTROLS_COUNT)
				selected_item++;
		}
		
		if(testKey(key1, key2, KEY_EXE))
		{
			button_pressed = true;

			if(selected_item == CONTROLS_COUNT)
			{
				in_menu = false; // close color palette menu
			}
			else
			{
				// edit key
				show_select_key_dialog(controls[selected_item]);
			}
		}
	}

	// restore lcd
	for(uint16_t y = 0; y < 528; y++)
	{
		for(uint16_t x = 0; x < 320; x++)
			vram[(y * 320) + x] = lcd_bak[y][x];
	}
}

void show_select_key_dialog(uint32_t *key_controls)
{	
	const uint16_t dialog_width = 100;
	const uint16_t dialog_height = 32;
	const uint16_t dialog_y = (528 - dialog_height) / 2;
	const uint16_t dialog_x = (320 - dialog_width) / 2;
	const uint16_t dialog_border = 0x04A0;
	const uint16_t dialog_bg = 0x2104;

	uint32_t key1;
	uint32_t key2;

	bool button_pressed = true;

	// draw dialog background
	for (uint16_t y = 0; y < dialog_height; y++)
	{
		for (uint16_t x = 0; x < dialog_width; x++)
		{
			if(y == 0 || y == (dialog_height - 1) || x == 0 || x == (dialog_width - 1))
				vram[((y + dialog_y) * (LCD_WIDTH * 2)) + x + dialog_x] = dialog_border;
			else
				vram[((y + dialog_y) * (LCD_WIDTH * 2)) + x + dialog_x] = dialog_bg;
		}
	}

	// draw title
	print_string("Press a key", dialog_x + ((dialog_width - (strlen("Press a key") * 6) - 2) / 2), 
		dialog_y + 10, 0, dialog_border, dialog_bg, 1);

	LCD_Refresh();

	// Wait for release
	while(button_pressed) 
	{ 
		getKey(&key1, &key2);

		if(!(key1 | key2))
			button_pressed = false;
	}

	getKey(&key1, &key2);
	
	while (!(key1 | key2))
		getKey(&key1, &key2);

	key_controls[0] = key1;
	key_controls[1] = key2;

	controls_changed = true;
}

void convert_byte_to_string(uint8_t byte, char *string)
{
	string[0] = (byte / 100) + '0';
	string[1] = ((byte % 100) / 10) + '0';
	string[2] = (byte % 10) + '0';
	string[3] = 0;

	if(byte < 10)
	{
		string[0] = string[2];	
		string[1] = 0;	
	}
	else if(byte < 100)
	{
		string[0] = string[1];	
		string[1] = string[2];	
		string[2] = 0;	
	}
}

uint8_t convert_string_to_byte(char *string)
{
	uint8_t byte = 0;
	uint8_t len = strlen(string);

	for(uint8_t i = 0; i < len; i++)
		byte = (byte * 10) + (string[i] - '0');

	return byte;
}

void create_palette(char *palette_name)
{
	// Debug_Printf(0, 4, 0, 0, "Creating palette");
	// for (uint8_t i = 0; i < 60; i++)
	// 	LCD_Refresh();

	struct palette new_palette;
	strcpy(new_palette.name, palette_name);
	memcpy(new_palette.data, default_palette, sizeof(default_palette));

	// Debug_Printf(0, 0, 0, 0, "Saving palette");
	// for (uint8_t i = 0; i < 60; i++)
	// 	LCD_Refresh();
	save_palette(&new_palette);
}

void delete_palette(struct palette *palette)
{
	char palette_path[200] = "\\fls0\\CPBoy\\palettes\\";
	strcat(palette_path, palette->name);
	strcat(palette_path, ".gbcp");

	remove(palette_path);
}

int8_t save_palette(struct palette *palette)
{
	// Debug_Printf(0, 1, 0, 0, "Started saving");
	// for (uint8_t i = 0; i < 60; i++)
	// 	LCD_Refresh();

	// make necessary directories
	mkdir("\\fls0\\CPBoy");
	mkdir("\\fls0\\CPBoy\\palettes");

	// Debug_Printf(0, 2, 0, 0, "Made directories");
	// for (uint8_t i = 0; i < 60; i++)
	// 	LCD_Refresh();

	char palette_path[200] = "\\fls0\\CPBoy\\palettes\\";
	strcat(palette_path, palette->name);
	strcat(palette_path, ".gbcp");

	// Debug_Printf(0, 3, 0, 0, palette_path);
	// for (uint8_t i = 0; i < 60; i++)
	// 	LCD_Refresh();
	
	int palette_file = open(palette_path, OPEN_CREATE | OPEN_WRITE);

	// Debug_Printf(0, 4, 0, 0, "Opened file");
	// for (uint8_t i = 0; i < 60; i++)
	// 	LCD_Refresh();
	
	if(palette_file < 0)
		return -1;

	// Debug_Printf(0, 5, 0, 0, "Open success");
	// for (uint8_t i = 0; i < 60; i++)
	// 	LCD_Refresh();
	
	if(write(palette_file, palette, sizeof(struct palette)) < 0)
	{
		close(palette_file);
		return -1;
	}
	// Debug_Printf(0, 6, 0, 0, "Close file");
	// for (uint8_t i = 0; i < 60; i++)
	// 	LCD_Refresh();

	close(palette_file);

	return 0;
}

void load_palettes()
{	
	char title[25];

	if(color_palettes != NULL)
		free(color_palettes);

	// check if gamepalette exists
	uint16_t game_palette[3][4];
	
	game_palette_exists = get_game_palette(gb_colour_hash(&gb), game_palette);

	palette_count = 1 + game_palette_exists;

	// get custom palette count
	const char palette_path[] = "\\fls0\\CPBoy\\palettes\\*.gbcp";
	wchar_t w_palette_path[sizeof(palette_path)];

	memset(w_palette_path, 0, sizeof(w_palette_path));
	
	for(uint8_t i = 0; palette_path[i] != 0; i++)
	{
		wchar_t ch = palette_path[i];
		w_palette_path[i] = ch;
	}

	wchar_t file_name[100];
	struct findInfo find_info;
	int find_handle;
	int ret = findFirst(w_palette_path, &find_handle, file_name, &find_info);

	// get file count
	while(ret >= 0) 
	{
		palette_count++;

		//serch next
		ret = findNext(find_handle, file_name, &find_info);
	}

	findClose(find_handle);

	color_palettes = (struct palette *)malloc(sizeof(struct palette) * palette_count);

	// Init default palette
	strcpy(color_palettes[0].name, "Default");
	memcpy(color_palettes[0].data, default_palette, sizeof(default_palette));

	// Add game default palette if it exists
	if(game_palette_exists)
	{		
		gb_get_rom_name(&gb, title);
		strcat(title, " Palette");

		strcpy(color_palettes[1].name, title);
		memcpy(color_palettes[1].data, game_palette, sizeof(game_palette));
	}

	// load custom palettes
	uint8_t palette_id = 1 + game_palette_exists;

	ret = findFirst(w_palette_path, &find_handle, file_name, &find_info);

	while(ret >= 0) 
	{
		char palette_file[200] = "\\fls0\\CPBoy\\palettes\\";
		char temp[100];
		uint8_t file_name_size = 0;

		//copy file name
		for(uint8_t i = 0; file_name[i] != 0; i++) 
		{
			wchar_t ch = file_name[i];
			temp[i] = ch;

			file_name_size++;
		}

		temp[file_name_size] = 0;

		strcat(palette_file, temp);

		// load this palette
		int f = open(palette_file, OPEN_READ);

		if(f >= 0)
		{
			read(f, &color_palettes[palette_id], sizeof(struct palette));
			close(f);
		}
		else
		{
			Debug_Printf(0, 1, 0, 0, "open() returned: %d", f);
			LCD_Refresh();			
		}

		//serch next
		ret = findNext(find_handle, file_name, &find_info);

		palette_id++;
	}

	findClose(find_handle);
}

int8_t save_controls(uint32_t (*controls_ptr)[2])
{
	// make user directory
	mkdir("\\fls0\\CPBoy");
	mkdir("\\fls0\\CPBoy\\user");

	// create save file
	int f = open("\\fls0\\CPBoy\\user\\controls.cpbc", OPEN_WRITE | OPEN_CREATE);

	if(f < 0)
		return -1;

	if(write(f, controls_ptr, sizeof(controls_ptr) * CONTROLS_COUNT) < 0)
	{
		close(f);
		return -1;
	}
	
	close(f);

	return 0;
}

void load_controls(uint32_t (*controls_ptr)[2])
{
	uint32_t default_controls[CONTROLS_COUNT][2] = 
	{
		{ 0, 					KEY_UP },
		{ 0, 					KEY_DOWN },
		{ KEY_LEFT, 	0 },
		{ KEY_RIGHT, 	0 },
		{ KEY_EXE, 		0 },
		{ KEY_ADD, 		0 },
		{ KEY_CLEAR, 	0 },
	};

	/*
	* We need to manually add the last element to the default controls array because
	* else the linker needs a ___movmem_i4_even subroutine (I don't know why)
	*/
	const uint32_t tmp[2] = { KEY_SHIFT, 	0 };
	memcpy(&default_controls[CONTROLS_COUNT - 1], tmp, sizeof(tmp));

	// init controls array
	memset(controls_ptr, 0, sizeof(controls_ptr) * CONTROLS_COUNT);

	// open save file
	int f = open("\\fls0\\CPBoy\\user\\controls.cpbc", OPEN_READ);

	if(f >= 0)
	{
		if(read(f, controls_ptr, sizeof(controls_ptr) * CONTROLS_COUNT) >= 0)
		{
			close(f);

			// check if all controls are set
			for (uint8_t i = 0; i < CONTROLS_COUNT; i++)
			{
				if(controls_ptr[i][0] | controls_ptr[i][1])
					continue;

				// if control is not set, use default controls
				controls_ptr[i][0] = default_controls[i][0];
				controls_ptr[i][1] = default_controls[i][1];
			}
			
			return;
		}
	
		close(f);
	}

	// load default controls
	memcpy(controls_ptr, default_controls, sizeof(default_controls));
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

void draw_slider(uint16_t x, uint16_t y, uint16_t width, uint16_t track_color,
	uint16_t handle_color, uint16_t max_value, uint16_t value)
{
	const uint16_t handle_height = 12;
	const uint16_t handle_width = 4;
	const uint16_t track_width = width - handle_width;

	// calculate handle position
	const uint16_t handle_offset = (((value * 0xFFFF) / max_value) * track_width) / 0xFFFF;

	// draw track
	draw_rectangle(x + (handle_width / 2), y + (handle_height / 2) - 1, track_width, 
		2, track_color, 0, 0);

	// draw handle
	draw_rectangle(x + handle_offset, y, handle_width, handle_height, handle_color, 0, 0);	
}

void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
	uint16_t color, uint16_t border, uint16_t border_color)
{
	for (uint16_t iy = 0; iy < height; iy++)
	{
		for (uint16_t ix = 0; ix < width; ix++)
		{
			uint16_t y_pixel = iy + y;
			uint16_t x_pixel = ix + x;

			if(y_pixel >= 528 || x_pixel >= 320)
				continue;

			uint16_t pixel_color = (ix < border || (width - ix - 1) < border || iy < border || (height - iy - 1) < border) * border_color + 
				!(ix < border || (width - ix - 1) < border || iy < border || (height - iy - 1) < border) * color;

			vram[(y_pixel * 320) + x_pixel] = pixel_color;
		}		
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