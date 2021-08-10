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

#include <cas-sdk/display.h>
#include <cas-sdk/file-system.h>
#include <cas-sdk/input/key-input.h>
#include "peanut_gb.h"

#define MAX_ROM_SIZE 0x10000

uint8_t *read_rom_to_ram(const char *file_name);
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr);
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr);
void error_print(char *message);
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val);
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val);
void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160], const uint_fast8_t line);

uint8_t rom[MAX_ROM_SIZE];

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;

	/* Colour palette for each BG, OBJ0, and OBJ1. */
	uint16_t selected_palette[3][4];
};

int main()
{
	/* 
		TODO: 
		 - implement rtc
		 - implement keyinput
	*/
	const char *rom_file_name = "/fls0/apps/gb-emu/rom.gb";

	enum gb_init_error_e gb_ret;
	struct gb_s gb;
	struct priv_t priv =
	{
		.rom = NULL,
		.cart_ram = NULL
	};

	clearScreen();
	printf("Loading ROM", 0, 0, 1);
	refreshDisplay();

	fatInitFileAccess();

	priv.rom = read_rom_to_ram(rom_file_name);

	if(priv.rom == NULL)
	{
		error_print("Error while reading ROM");
		return 1;
	}

	clearScreen();
	printf("Init", 0, 0, 1);
	refreshDisplay();

	/* Initialise emulator context. */
	gb_ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
		&gb_error, &priv);

	switch(gb_ret)
	{
		case GB_INIT_NO_ERROR:
			break;

		case GB_INIT_CARTRIDGE_UNSUPPORTED:
			error_print("Unsupported cartridge");
			return 1;

		case GB_INIT_INVALID_CHECKSUM:
			error_print("ROM checksum failure");
			return 1;

		default:
			error_print("Unknown error on init");
			return 1;
	}

	/* Initialise lcd stuff */
	gb_init_lcd(&gb, &lcd_draw_line);

	const uint16_t palette[3][4] =
	{
		{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
		{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
		{ 0x7FFF, 0x5294, 0x294A, 0x0000 }
	};
	memcpy(priv.selected_palette, palette, sizeof(palette));

	clearScreen();
	printf("Init complete", 0, 0, 1);
	refreshDisplay();

	while (1)
	{
		/* Handle Key Input */

		/* Run CPU until next frame */
		gb_run_frame(&gb);

		/* Update screen with current frame data */
		refreshDisplay();
	}

	return 0;
}

void error_print(char *message)
{
	clearScreen();
	printf(message, 0, 0, 1);
	
	for (uint8_t i = 0; i < 100; i++)
		refreshDisplay();	
}

uint8_t *read_rom_to_ram(const char *file_name)
{
	int32_t rom_file = fatOpenFile(file_name, OPEN_READ);

	if(rom_file < 0)
		return NULL;
		
	int32_t status = fatReadFile(rom_file, rom, sizeof(rom));

	printHexWord(status>>16, 0, 3);
	printHexWord(status, 5, 3);
	
	if(status < 0)
		return NULL;

	fatCloseFile(rom_file);

	return rom;
}

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, 
	const uint8_t val)
{
	const struct priv_t * const p = gb->direct.priv;
	p->cart_ram[addr] = val;
}

/**
 * Handles an error reported by the emulator. The emulator context may be used
 * to better understand why the error given in gb_err was reported.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val)
{
	struct priv_t *priv = gb->direct.priv;

	switch(gb_err)
	{
		case GB_INVALID_OPCODE:
			error_print("Invalid opcode");
			// fprintf(stdout, "Invalid opcode %#04x at PC: %#06x, SP: %#06x\n",
			// 	val,
			// 	gb->cpu_reg.pc - 1,
			// 	gb->cpu_reg.sp);
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
	struct priv_t *priv = gb->direct.priv;

	for(unsigned int x = 0; x < LCD_WIDTH; x++)
	{
		uint16_t color = priv->selected_palette
						[(pixels[x] & LCD_PALETTE_ALL) >> 4]
				    [pixels[x] & 3];

		frameBuffer[line * 2][x * 2] = color;
		frameBuffer[line * 2][(x * 2) + 1] = color;
		frameBuffer[(line * 2) + 1][x * 2] = color;
		frameBuffer[(line * 2) + 1][(x * 2) + 1] = color;
	}
}

int main() __attribute__((section(".text.main")));
