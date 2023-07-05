#include "cart_ram.h"

#include <stdlib.h>
#include <sdk/os/file.hpp>
#include "error.h"
#include "emulator.h"
#include "../helpers/macros.h"
#include "../helpers/fileio.h"
#include "../helpers/functions.h"

#define RAM_VAR_CONSTANT  "R_"

// Gets the filename of the current roms cart ram save
// Make sure the buffer is big enough
void get_cart_ram_file_name(emu_preferences *preferences, char *name_buffer)
{	
  strcpy(name_buffer, DIRECTORY_CARTRAM "\\");
	strcat(name_buffer, preferences->current_rom_name);	
	strcat(name_buffer, EXTENSION_CARTRAM);
}

// Gets the mcs varname of the current roms cart ram save
// Make sure the buffer is big enough
char *get_cart_ram_var_name(emu_preferences *preferences, char *name_buffer)
{	
  char *str = preferences->current_rom_name;

  // Check if this rom has a name
  if (strcmp(preferences->current_rom_name, "") == 0) 
  {
    str = preferences->current_filename;
  } 

	strcpy(name_buffer, RAM_VAR_CONSTANT);
  itoa_leading_zeros(
    hash_string(str, 0xFFFFFF), 
    name_buffer + (sizeof(RAM_VAR_CONSTANT) - 1),
    16,
    6
  );

  return name_buffer;
}

uint8_t load_cart_ram(struct gb_s *gb, size_t len)
{
  emu_preferences *preferences = (emu_preferences *)(gb->direct.priv);

	// If save file not required.
	if (len == 0)
	{
		preferences->cart_ram = nullptr;
		return 0;
	}

	// Allocate enough memory to hold save file.
	preferences->cart_ram = (uint8_t *)malloc(len);

	if(!preferences->cart_ram) 
  {
    char err_info[ERROR_MAX_INFO_LEN];
    char tmp[20];

    strlcpy(err_info, "Cart RAM: ", sizeof(err_info));
    strlcat(err_info, itoa(len, tmp, 10), sizeof(err_info));
    strlcat(err_info, "B", sizeof(err_info));
    
    set_error_i(EMALLOC, err_info);
		return 1;
  }
  
  memset(preferences->cart_ram, 0xFF, len);

	// Load cart ram
	char var_name[MAX_FILENAME_LEN];
  char *mcs_data = nullptr;
  uint32_t mcs_size = 0;

	get_cart_ram_var_name(preferences, var_name);

  // If succesfully read data, convert copy it to cart ram
  if (read_mcs(MCS_DIRECTORY, var_name, (void **)&mcs_data, &mcs_size) || !mcs_data)
  {
    return 1;
  }

  // Convert to byte array
  for (uint32_t i = 0; i < mcs_size; i += 2)
  {
    char hex_byte[3] = { mcs_data[i], mcs_data[i + 1], '\0'};
    preferences->cart_ram[i / 2] = strtol(hex_byte, nullptr, 16);
  }

  return 0;	
}

uint8_t save_cart_ram(struct gb_s *gb, size_t len)
{
  emu_preferences *preferences = (emu_preferences *)(gb->direct.priv);

  // Current ROM does not have a cart ram
	if(len == 0 || !(preferences->cart_ram))
  {
		return 0;
  }

  // Convert byte array to hex string array
  const uint32_t str_size = (len * 2) + 1;
  const uint32_t var_size = align_val(str_size + 1, 4);
  char var_contents[var_size];
  
  memset(var_contents, 0, var_size);

  for (size_t i = 0; i < len; i++)
  {  
    itoa_leading_zeros(preferences->cart_ram[i], &(var_contents[(i * 2)]), 16, 2);
  }

  // Add trailing 0 just to make sure
  var_contents[str_size - 1] = '\0';

	// Write cart rom
	char var_name[MAX_FILENAME_LEN];
	get_cart_ram_var_name(preferences, var_name);

	return write_mcs(MCS_DIRECTORY, var_name, var_contents, var_size);	
}
