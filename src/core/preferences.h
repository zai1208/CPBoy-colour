#pragma once

#include <stdint.h>
#include "controls.h"
#include "palettes.h"
#include "../helpers/macros.h"

#define DEFAULT_INTERLACE_ENABLE  false
#define DEFAULT_FRAMESKIP_ENABLE  false
#define DEFAULT_FRAMESKIP_AMOUNT  1
#define DEFAULT_EMU_SPEED         100
#define DEFAULT_SELECTED_PALETTE  0

typedef struct 
{
  bool interlacing_enabled;

  bool frameskip_enabled;
  uint8_t frameskip_amount;

  uint16_t emulation_speed;
  
  uint8_t selected_palette;
} rom_config;

typedef struct 
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;

  char current_filename[200];
  char current_rom_name[16];

  bool emulator_paused;

  palette *palettes;
  uint8_t palette_count;

  emu_controls controls;
  rom_config config;

  struct 
  {
    bool controls_changed;
    bool rom_config_changed;
  } file_states;
} emu_preferences;

uint8_t load_rom_config(struct gb_s *gb);

uint8_t save_rom_config(struct gb_s *gb);
