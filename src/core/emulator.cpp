#include "emulator.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sdk/calc/calc.hpp>
#include <sdk/os/file.hpp>
#include <sdk/os/input.hpp>
#include <sdk/os/debug.hpp>
#include "cart_ram.h"
#include "cas_display.h"
#include "controls.h"
#include "error.h"
#include "peanut_gb.h"
#include "../emu_ui/menu/menu.h"
#include "../helpers/macros.h"
#include "../helpers/functions.h"
#include "../helpers/fileio.h"

#define INPUT_NONE      0
#define INPUT_OPEN_MENU 1

uint8_t execution_handle_input(struct gb_s *gb)
{
  // Skip this function if no keys are pressed
  if (!Input_IsAnyKeyDown())
  {
    // Unpress all buttons
    gb->direct.joypad = 0xFF;

    return INPUT_NONE;
  }

  uint32_t key1;
  uint32_t key2;

  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Handle Key Input
  getKey(&key1, &key2);

  gb->direct.joypad_bits.a =      !((key1 & preferences->controls[GB_KEY_A][0])       | (key2 & preferences->controls[GB_KEY_A][1]));
  gb->direct.joypad_bits.b =      !((key1 & preferences->controls[GB_KEY_B][0])       | (key2 & preferences->controls[GB_KEY_B][1]));
  gb->direct.joypad_bits.start =  !((key1 & preferences->controls[GB_KEY_START][0])   | (key2 & preferences->controls[GB_KEY_START][1]));
  gb->direct.joypad_bits.select = !((key1 & preferences->controls[GB_KEY_SELECT][0])  | (key2 & preferences->controls[GB_KEY_SELECT][1]));
  gb->direct.joypad_bits.up =     !((key1 & preferences->controls[GB_KEY_UP][0])      | (key2 & preferences->controls[GB_KEY_UP][1]));
  gb->direct.joypad_bits.down =   !((key1 & preferences->controls[GB_KEY_DOWN][0])    | (key2 & preferences->controls[GB_KEY_DOWN][1]));
  gb->direct.joypad_bits.left =   !((key1 & preferences->controls[GB_KEY_LEFT][0])    | (key2 & preferences->controls[GB_KEY_LEFT][1]));
  gb->direct.joypad_bits.right =  !((key1 & preferences->controls[GB_KEY_RIGHT][0])   | (key2 & preferences->controls[GB_KEY_RIGHT][1]));

  // Check if menu should be opened
  if(testKey(key1, key2, KEY_NEGATIVE))
  {
    return INPUT_OPEN_MENU;
  }

  return INPUT_NONE;
}

void set_frameskip(struct gb_s *gb, bool enabled, uint8_t amount)
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  uint8_t new_amount = clamp(amount, (uint8_t)FRAMESKIP_MIN, (uint8_t)FRAMESKIP_MAX);

  // Check if anything should be changed
  if (enabled == preferences->config.frameskip_enabled && new_amount == preferences->config.frameskip_amount)
  {
    return;
  }

  gb->direct.frame_skip = enabled;
  gb->direct.frame_skip_amount = new_amount;
  preferences->config.frameskip_enabled = enabled;
  preferences->config.frameskip_amount = new_amount;    

  preferences->file_states.rom_config_changed = true;
}

void set_interlacing(struct gb_s *gb, bool enabled)
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Check if anything should be changed
  if (enabled == preferences->config.interlacing_enabled)
  {
    return;
  }
  
  gb->direct.interlace = enabled;
  preferences->config.interlacing_enabled = enabled;

  preferences->file_states.rom_config_changed = true;
}

// Draws scanline into framebuffer.
void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160],
  const uint_fast8_t line)
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  palette selected_palette = preferences->palettes[preferences->config.selected_palette];

  for(uint16_t x = 0; x < LCD_WIDTH; x++)
  {
    uint16_t color = selected_palette.data
      [(pixels[x] & LCD_PALETTE_ALL) >> 4]
      [pixels[x] & 3];

    vram[(line * (LCD_WIDTH * 4)) + (x * 2)] = color;
    vram[(line * (LCD_WIDTH * 4)) + (x * 2) + 1] = color;
    vram[(line * (LCD_WIDTH * 4)) + (LCD_WIDTH * 2) + (x * 2)] = color;
    vram[(line * (LCD_WIDTH * 4)) + (LCD_WIDTH * 2) + (x * 2) + 1] = color;
  }
}

// Returns a byte from the ROM file at the given address.
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
  const emu_preferences *const p = (emu_preferences *)gb->direct.priv;
  return p->rom[addr];
}

// Returns a byte from the cartridge RAM at the given address.
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
  const emu_preferences *const p = (emu_preferences *)gb->direct.priv;
  return p->cart_ram[addr];
}

// Writes a given byte to the cartridge RAM at the given address.
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, 
  const uint8_t val)
{
  const emu_preferences *const p = (emu_preferences *)gb->direct.priv;
  p->cart_ram[addr] = val;
}

// Handles an error reported by the emulator. The emulator context may be used
// to better understand why the error given in gb_err was reported.
// TODO: Correctly implement this
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val)
{
  switch(gb_err)
  {
    case GB_INVALID_OPCODE:
      Debug_Printf(0, 0, false, 0, "Invalid opcode %#04x at PC: %#06x, SP: %#06x\n",
        val,
        gb->cpu_reg.pc.reg - 1,
        gb->cpu_reg.sp);
      break;

    // Ignoring non fatal errors.
    case GB_INVALID_WRITE:
    case GB_INVALID_READ:
      Debug_Printf(0, 0, false, 0, "IO-Operation failed");
      return;

    default:
      Debug_Printf(0, 0, false, 0, "Unknown error while executing");
      break;
  }
  
  for (uint8_t i = 0; i < 100; i++) 
  {
    LCD_Refresh();	
  }

  return;
}

uint8_t prepare_emulator(struct gb_s *gb, emu_preferences *preferences)
{
  enum gb_init_error_e gb_ret;

  // Initialise emulator context
  gb_ret = gb_init(gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
    &gb_error, preferences);
  
  // Add ROM name to preference struct
  gb_get_rom_name(gb, preferences->current_rom_name);

  switch(gb_ret)
  {
    case GB_INIT_NO_ERROR:
      break;

    case GB_INIT_CARTRIDGE_UNSUPPORTED:
      set_error(EEMUCARTRIDGE);
      return 1;

    case GB_INIT_INVALID_CHECKSUM:
      set_error(EEMUCHECKSUM);
      return 1;

    default:
      set_error(EEMUGEN);
      return 1;
  }

  // Init gameboy rtc (Just zero everything)
  struct tm time;
  time.tm_sec = 0;
  time.tm_min = 0;
  time.tm_hour = 0;
  time.tm_yday = 0;

  gb_set_rtc(gb, &time);

  // Initialise lcd stuff 
  gb_init_lcd(gb, &lcd_draw_line);

  // Load cart save
  load_cart_ram(gb, gb_get_save_size(gb));

  // Load user configs
  load_rom_config(gb);
  load_controls(gb);

  preferences->file_states.controls_changed = false;
  preferences->file_states.rom_config_changed = false;
  
  if (load_palettes(gb))
  {
    return 1;
  }

  // load_savestates();

  // check if loaded palette is out of range
  if(preferences->config.selected_palette >= preferences->palette_count) 
  {
    // Reset to default palette
    preferences->config.selected_palette = 0;
  }

  return 0;
}

void free_emulator(struct gb_s *gb)
{
  emu_preferences *prefs = (emu_preferences *)gb->direct.priv;

  free(prefs->rom);
  free(prefs->cart_ram);
  free(prefs->palettes);
}

uint8_t close_rom(struct gb_s *gb)
{
  emu_preferences *prefs = (emu_preferences *)gb->direct.priv;
  uint8_t return_code = save_cart_ram(gb, gb_get_save_size(gb));

  if (prefs->file_states.rom_config_changed)
  {
    return_code |= save_rom_config(gb);
  }    

  free_emulator(gb);

  return return_code;
}

uint8_t execute_rom(struct gb_s *gb) 
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  uint32_t frame = 0;
  uint8_t frames_skipped = 0;

  for (;;)
  {
    frame++;

    // Handle rtc
    if (frame % 24 == 0)
    {
      gb_tick_rtc(gb);
    }

    // Run CPU until next frame
    gb_run_frame(gb);

    // Handle input
    if (execution_handle_input(gb) == INPUT_OPEN_MENU)
    {
      uint8_t menu_code = emulation_menu(gb, false);

      if (menu_code != MENU_CLOSED)
      {
        return menu_code;
      }

      LCD_Refresh();
    }

    // Check if display should be updated
    if(gb->direct.frame_skip)
    {
      if (frames_skipped < preferences->config.frameskip_amount)
      {
        frames_skipped++;
        continue;
      }
      else 
      {
        frames_skipped = 0;
      }
    }

    // Update screen with current frame data
    // Only refresh actual gameboy screen
    refresh_gb_lcd(0, CAS_LCD_WIDTH, 0, LCD_HEIGHT * 2);
  }
}

uint8_t run_emulator(struct gb_s *gb, emu_preferences *prefs) 
{
  bool exit_emulator = false;

  // Show load menu
  switch (load_menu(prefs))
  {
    case MENU_EMU_QUIT:
      return 0;

    case MENU_CRASH:
      return 1;
    
    default:
      break;
  }

  // Emulator rom load, execute and unload loop
  while (!exit_emulator)
  {
    draw_load_alert();
    LCD_Refresh();

    if (load_rom(prefs) != 0)
    {
      return 1;
    }

    if (prepare_emulator(gb, prefs) != 0)
    {
      return 1;
    }    
    
    // Render preview of menu
    fillScreen(0x0000);
    emulation_menu(gb, true);
    LCD_Refresh();
    
    switch (execute_rom(gb))
    {
      case MENU_CRASH:
        // Try to normaly close the rom, but this may fail
        close_rom(gb);
        return 1;

      case MENU_EMU_QUIT:
        exit_emulator = true;

      default:
        break;
    }

    if (close_rom(gb) != 0)
    {
      return 1;
    }
  }

  if (prefs->file_states.controls_changed)
  {
    if (save_controls(gb) != 0)
    {
      return 1;
    }
  }    
  
  return 0;
}

uint8_t load_rom(emu_preferences *prefs)
{
  char rom_filename[MAX_FILENAME_LEN] = DIRECTORY_ROM "\\";
  strncat(rom_filename, prefs->current_filename, MAX_FILENAME_LEN);
  rom_filename[MAX_FILENAME_LEN - 1] = '\0';
  
  // Open file, get size and allocate memory for reading
	int32_t rom_file = open(rom_filename, OPEN_READ);	
	
	if (rom_file < 0)
  {
    set_error_i(EFOPEN, rom_filename);
		return 1;
  }

	struct stat rom_file_stat;

	if (fstat(rom_file, &rom_file_stat) < 0)
  {
    set_error(EFREAD);
		return 1;
  }

	if (close(rom_file) < 0)
  {
    set_error(EFCLOSE);
		return 1;
  }

	// dynamically allocate space for rom in heap
	prefs->rom = (uint8_t *)malloc(rom_file_stat.fileSize);

	// check if pointer to rom is no nullptr
	if (!prefs->rom)
  {
    char err_info[ERROR_MAX_INFO_LEN];
    char tmp[20];

    strlcpy(err_info, "GB ROM: ", sizeof(err_info));
    strlcat(err_info, itoa(rom_file_stat.fileSize, tmp, 10), sizeof(err_info));
    strlcat(err_info, "B", sizeof(err_info));
    
    set_error_i(EMALLOC, err_info);
		return 1;
  }

  if(read_file(rom_filename, prefs->rom, rom_file_stat.fileSize) != 0)
  {
    return 1;
  }

  return 0;
}
