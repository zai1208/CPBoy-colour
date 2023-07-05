#pragma once

#include <stdint.h>

#define MAX_PALETTE_NAME_LEN        30
#define MAX_PALETTE_COUNT           10

#define ERR_MAX_PALETTE_REACHED     2

#define DEFAULT_PALETTE \
{ \
  { 0x7FFF, 0x5294, 0x294A, 0x0000 }, \
  { 0x7FFF, 0x5294, 0x294A, 0x0000 }, \
  { 0x7FFF, 0x5294, 0x294A, 0x0000 } \
}

typedef struct
{
	char name[MAX_PALETTE_NAME_LEN] __attribute__((aligned));

	// The actual content of the palette 
	uint16_t data[3][4] __attribute__((aligned));
} palette;

uint8_t create_palette(struct gb_s *gb);

uint8_t delete_palette(struct gb_s *gb, uint8_t index);

uint8_t load_palettes(struct gb_s *gb);

uint8_t save_palette(palette *pal, uint8_t index);

uint8_t get_user_palettes(palette **pal, struct gb_s *gb);