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
#include "peanut_gb.h"

// #define MAX_ROM_SIZE 0x10000

#define KEY_UP				0
#define KEY_DOWN			1
#define KEY_LEFT			2
#define KEY_RIGHT			3
#define KEY_PLUS			4
#define KEY_SHIFT			5
#define KEY_CLEAR			6
#define KEY_EXE				7
#define KEY_KEYBOARD	8		
#define KEY_BACKSPACE	9			
#define KEY_NEGATIVE	10			

/*
 * Fill this section in with some information about your app.
 * All fields are optional - so if you don't need one, take it out.
 */
APP_NAME("CPBoy")
APP_DESCRIPTION("A Gameboy (DMG) emulator. Forked from PeanutGB by deltabeard.")
APP_AUTHOR("diddyholz")
APP_VERSION("0.0.1-alpha")

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
uint8_t initEmulator();
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
	ScancodeNegative
};

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;

	/* Colour palette for each BG, OBJ0, and OBJ1. */
	uint16_t selected_palette[3][4];
};

struct gb_s gb;
struct priv_t priv =
{
	.rom = NULL,
	.cart_ram = NULL
};

extern "C"
void main()
{
	/* 
		TODO: 
		 - implement rtc
		 - implement keyinput
	*/
	calcInit(); //backup screen and init some variables

	const char *rom_file_name = "\\fls0\\rom.gb";

	// make save directory
	mkdir("\\fls0\\gb-saves");

	LCD_ClearScreen();
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

	calcEnd();
}

uint8_t initEmulator()
{
	LCD_ClearScreen();
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

	const uint16_t palette[3][4] =
	{
		{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
		{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
		{ 0x7FFF, 0x5294, 0x294A, 0x0000 }
	};
	memcpy(priv.selected_palette, palette, sizeof(palette));

	LCD_ClearScreen();
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
		if(frame % 60 == 0)
			gb_tick_rtc(&gb);

		/* Handle Key Input */
		if(frame % 10 == 0)
		{
			gb.direct.joypad_bits.a = !Input_GetKeyState(&scancodes[KEY_EXE]);
			gb.direct.joypad_bits.b = !Input_GetKeyState(&scancodes[KEY_PLUS]);
			gb.direct.joypad_bits.select = !Input_GetKeyState(&scancodes[KEY_SHIFT]);
			gb.direct.joypad_bits.start = !Input_GetKeyState(&scancodes[KEY_CLEAR]);
			gb.direct.joypad_bits.up = !Input_GetKeyState(&scancodes[KEY_UP]);
			gb.direct.joypad_bits.down = !Input_GetKeyState(&scancodes[KEY_DOWN]);
			gb.direct.joypad_bits.left = !Input_GetKeyState(&scancodes[KEY_LEFT]);
			gb.direct.joypad_bits.right = !Input_GetKeyState(&scancodes[KEY_RIGHT]);

			if(Input_GetKeyState(&scancodes[KEY_KEYBOARD]))
			{
				gb.direct.frame_skip = !gb.direct.frame_skip;

				if(gb.direct.frame_skip)
					error_print("Frameskip on");
				else
					error_print("Frameskip off");
			}

			if(Input_GetKeyState(&scancodes[KEY_BACKSPACE]))
			{
				gb.direct.interlace = !gb.direct.interlace;

				if(gb.direct.interlace)
					error_print("Interlace on");
				else
					error_print("Interlace off");
			}
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

		if(Input_GetKeyState(&scancodes[KEY_NEGATIVE]))
			return;

		/* Update screen with current frame data */
		LCD_Refresh(); 
		frame++;
	}
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
	uint8_t *rom = new uint8_t[rom_file_stat.fileSize];

	// check if pointer to rom is no nullptr
	if(!rom)
		return NULL;
		
	int32_t status = read(rom_file, rom, rom_file_stat.fileSize);
	
	if(status < 0)
		return NULL;

	close(rom_file);

	return rom;
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
	*dest = new uint8_t[len];

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
