#include "palettes.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sdk/os/file.hpp>
#include <sdk/os/debug.hpp>
#include "error.h"
#include "emulator.h"
#include "../helpers/fileio.h"
#include "../helpers/functions.h"
#include "../helpers/ini.h"
#include "../helpers/macros.h"
#include "../emu_ui/font.h"

#define PALETTE_NAME_CONSTANT         "Palette "
#define PALETTE_VAR_CONSTANT          "Pal"

#define PALETTE_CONF_VAR_NAME         "PalCfg"
#define PALETTE_CONF_INI_SECTION_NAME "PalCfg"
#define PALETTE_CONF_INI_COUNT_KEY    "pal_cnt"

#define PALETTE_INI_SECTION_NAME      "Pal"
#define PALETTE_INI_NAME_KEY          "name"
#define PALETTE_INI_KEY_SEPERATOR     '_'

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

char *generate_palette_name(char *str_buffer, struct gb_s *gb)
{
  // Go through all palettes and check which index is free
  palette *user_palettes;
  uint8_t user_palette_count = get_user_palettes(&user_palettes, gb);
  int8_t max_index = -1;

  for (uint8_t i = 0; i < user_palette_count; i++) 
  {
    char *p = strchr(user_palettes[i].name, ' ');

    if (!p) 
    {
      continue;
    }

    int tmp = atoi(p + 1);

    if (tmp > max_index) 
    {
      max_index = tmp;
    }
  }

  strcpy(str_buffer, PALETTE_NAME_CONSTANT);

  char tmp[4];

  strcat(str_buffer, itoa_leading_zeros(max_index + 1, tmp, 10, 2));

  return str_buffer;
}

uint8_t process_palette_ini(char *ini_string, uint32_t len, palette *pal)
{
  ini_file file;
  ini_parse(ini_string, len, &file);

  ini_section *section = find_section(&file, PALETTE_INI_SECTION_NAME);

  if (!section)
  {
    free_ini_file(&file);
    return 1;
  }

  ini_key *name = find_key(section, PALETTE_INI_NAME_KEY);
      
  if (!name) 
  {
    free_ini_file(&file);
    return 1;
  }

  strcpy(pal->name, name->value_str);
  
  // Get all colors
  for (uint8_t p = 0; p < 3; p++) 
  {
    for (uint8_t c = 0; c < 4; c++) 
    {
      char a = '0' + p;
      char b = '0' + c;

      const char key_name[4] = { a, PALETTE_INI_KEY_SEPERATOR, b, '\0' };

      ini_key *color = find_key(section, key_name);
      
      if (!color) 
      {
        free_ini_file(&file);
        return 1;
      }

      pal->data[p][c] = color->value_int;
    }
  }

  free_ini_file(&file);
  
  return 0;
}

char *create_palette_ini(palette *pal, char *ini_string, uint32_t len)
{
  ini_file file;
  file.section_count = 0;
  file.sections_size = 0;
  
  ini_section *palette_section = add_section(&file, PALETTE_INI_SECTION_NAME);
  
  if (!palette_section)
  {
    return nullptr;
  }

  if (!add_key(
    palette_section,
    PALETTE_INI_NAME_KEY,
    INI_TYPE_STRING,
    0,
    pal->name
  ))
  {
    free_ini_file(&file);
    return nullptr;
  }

  // Setup palette data
  for (uint8_t p = 0; p < 3; p++)
  {
    for (uint8_t c = 0; c < 4; c++)
    {
      char a = '0' + p;
      char b = '0' + c;

      const char key_name[] = { a, PALETTE_INI_KEY_SEPERATOR, b, '\0' };
      
      if (!add_key(
        palette_section,
        key_name,
        INI_TYPE_INT,
        pal->data[p][c]
      ))
      {
        free_ini_file(&file);
        return nullptr;
      }
    }
  }

  ini_write(&file, ini_string, len);
  free_ini_file(&file);

  return ini_string;
}

uint8_t process_palette_config_ini(char *ini_string, uint32_t len, uint8_t *count)
{
  ini_file file;
  ini_parse(ini_string, len, &file);

  ini_section *section = find_section(&file, PALETTE_CONF_INI_SECTION_NAME);

  if (!section)
  {
    free_ini_file(&file);
    return 1;
  }

  ini_key *pal_count = find_key(section, PALETTE_CONF_INI_COUNT_KEY);
      
  if (!pal_count) 
  {
    free_ini_file(&file);
    return 1;
  }

  *count = pal_count->value_int;

  free_ini_file(&file);
  
  return 0;
}

char *create_palette_config_ini(uint8_t count, char *ini_string, uint32_t len)
{
  ini_file file;
  file.section_count = 0;
  file.sections_size = 0;
  
  ini_section *pal_config_section = add_section(&file, PALETTE_CONF_INI_SECTION_NAME);
  
  if (!pal_config_section)
  {
    free_ini_file(&file);
    return nullptr;
  }

  if (!add_key(
    pal_config_section, 
    PALETTE_CONF_INI_COUNT_KEY,
    INI_TYPE_INT,
    count
  ))
  {
    free_ini_file(&file);
    return nullptr;
  }
  
  ini_write(&file, ini_string, len);
  free_ini_file(&file);

  return ini_string;
}

uint8_t save_palette_config(uint8_t pal_count)
{
  char ini_string[INI_MAX_CONTENT_LEN];

  if (!create_palette_config_ini(pal_count, ini_string, sizeof(ini_string)))
  {
    return 1;
  }

  // Get 4 aligned size for mcs variable
  uint32_t size = align_val(strlen(ini_string), 4);

  if (write_mcs(MCS_DIRECTORY, PALETTE_CONF_VAR_NAME, ini_string, size))
  {
    return 1;
  }

  return 0;
}

uint8_t create_palette(struct gb_s *gb) 
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
 
  palette new_pal;
  uint16_t default_palette[3][4] = DEFAULT_PALETTE;
  uint8_t user_palette_count = get_user_palettes(nullptr, gb);
 
  // Check if already reached max palette count
  if (user_palette_count == MAX_PALETTE_COUNT) 
  {
    return ERR_MAX_PALETTE_REACHED;
  }

  generate_palette_name(new_pal.name, gb);
  memcpy(new_pal.data, default_palette, sizeof(default_palette));

  // Save newly created palette
  if (save_palette(&new_pal, user_palette_count))
  {
    return 1;
  }

  // Save new palette config
  if (save_palette_config(user_palette_count + 1))
  {
    return 1;
  }

  // Reload palettes
  free(preferences->palettes);
  load_palettes(gb);

  return 0;
}

uint8_t delete_palette(struct gb_s *gb, uint8_t index) 
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  palette *user_palettes = nullptr;
  uint8_t user_palette_count = get_user_palettes(&user_palettes, gb);

  // Overwrite palette in palette array and move everything to the front
  for (uint8_t i = index; i < (user_palette_count - 1); i++) 
  {
    memcpy(&(user_palettes[i]), &(user_palettes[i + 1]), sizeof(palette));
    save_palette(&(user_palettes[i]), i);
  }

  // Reduce total palette count
  preferences->palette_count--;

  save_palette_config(user_palette_count - 1);

  return 0;
}

uint8_t load_palettes(struct gb_s *gb)
{
  emu_preferences *preferences = (emu_preferences *)(gb->direct.priv);

  uint32_t var_size;
  uint8_t user_palette_count = 0;
  char *ini_string;

  // Try to load palette config
  if (read_mcs(MCS_DIRECTORY, PALETTE_CONF_VAR_NAME, (void **)&ini_string, &var_size) == 0)
  {
    if (process_palette_config_ini(ini_string, var_size, &user_palette_count))
    {
      return 1;
    }
  }
  
  // Get game palette
	uint16_t game_palette[3][4];

  bool has_game_palette = get_game_palette(gb_colour_hash(gb), game_palette);

  preferences->palette_count = 1 + has_game_palette + user_palette_count;
  preferences->palettes = (palette *)malloc(preferences->palette_count * sizeof(palette));

  // Check if malloc failed
  if (!(preferences->palettes))
  {
    char tmp[10];
    char err_info[ERROR_MAX_INFO_LEN];
    strlcpy(err_info, "Palettes: ", sizeof(err_info));
    strlcat(err_info, itoa(preferences->palette_count, tmp, 10), sizeof(err_info));
    strlcat(err_info, "B", sizeof(err_info));

    set_error_i(EMALLOC, err_info);
    return 1;
  }

  // Set default palette
  uint16_t default_palette[3][4] = DEFAULT_PALETTE;
  strlcpy(preferences->palettes[0].name, "Default", sizeof(preferences->palettes[0].name));
  memcpy(preferences->palettes[0].data, default_palette, sizeof(default_palette));

  // Set game palette if it exists
  if (has_game_palette)
  {
    strlcpy(preferences->palettes[1].name, preferences->current_rom_name, sizeof(preferences->palettes[1].name));
    memcpy(preferences->palettes[1].data, game_palette, sizeof(game_palette));
  }

  // Set user palettes
  for (uint8_t i = 0; i < user_palette_count; i++)
  {
    char tmp[3];
    char var_name[sizeof(PALETTE_VAR_CONSTANT) + 2] = PALETTE_VAR_CONSTANT;
    
    strlcat(var_name, itoa(i, tmp, 16), sizeof(var_name));

    if (read_mcs(MCS_DIRECTORY, var_name, (void **)&ini_string, &var_size))
    {
      return 1;
    }

    if (process_palette_ini(ini_string, var_size, &(preferences->palettes[1 + has_game_palette + i]))) 
    {
      return 1;
    }
  }

  return 0;
}

uint8_t save_palette(palette *pal, uint8_t index)
{
  char ini_string[INI_MAX_CONTENT_LEN];

  if (!create_palette_ini(pal, ini_string, sizeof(ini_string)))
  {
    return 1;
  }

  uint32_t size = align_val(strlen(ini_string), 4);

  char tmp[3];
  char var_name[sizeof(tmp) + sizeof(PALETTE_VAR_CONSTANT) - 1] = PALETTE_VAR_CONSTANT;
  
  strlcat(var_name, itoa(index, tmp, 16), sizeof(var_name));

  if (write_mcs(MCS_DIRECTORY, var_name, ini_string, size) != 0)
  {
    return 1;
  }

  return 0;
}

uint8_t get_user_palettes(palette **pal, struct gb_s *gb)
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  uint16_t tmp[3][4];

  bool has_game_palette = get_game_palette(gb_colour_hash(gb), tmp);

  if (pal)
  {
    *pal = &(preferences->palettes[1 + has_game_palette]);
  }

  return preferences->palette_count - 1 - has_game_palette;
}
