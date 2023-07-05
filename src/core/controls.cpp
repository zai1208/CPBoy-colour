#include "controls.h"

#include <sdk/os/file.hpp>
#include <sdk/os/debug.hpp>
#include <sdk/calc/calc.hpp>
#include "preferences.h"
#include "emulator.h"
#include "error.h"
#include "../helpers/fileio.h"
#include "../helpers/functions.h"
#include "../helpers/ini.h"
#include "../helpers/macros.h"

#define CONTROLS_INI_VAR_NAME       "Controls"

#define CONTROLS_INI_SECTION_NAME   "Ctrls"
#define CONTROLS_INI_KEY_SEPERATOR  '_'

void set_controls_defaults(emu_controls *controls)
{
  (*controls)[GB_KEY_A][0] = DEFAULT_GB_KEY_A_0;
  (*controls)[GB_KEY_A][1] = DEFAULT_GB_KEY_A_1;
  (*controls)[GB_KEY_B][0] = DEFAULT_GB_KEY_B_0;
  (*controls)[GB_KEY_B][1] = DEFAULT_GB_KEY_B_1;
  (*controls)[GB_KEY_START][0] = DEFAULT_GB_KEY_START_0;
  (*controls)[GB_KEY_START][1] = DEFAULT_GB_KEY_START_1;
  (*controls)[GB_KEY_SELECT][0] = DEFAULT_GB_KEY_SELECT_0;
  (*controls)[GB_KEY_SELECT][1] = DEFAULT_GB_KEY_SELECT_1;
  (*controls)[GB_KEY_UP][0] = DEFAULT_GB_KEY_UP_0;
  (*controls)[GB_KEY_UP][1] = DEFAULT_GB_KEY_UP_1;
  (*controls)[GB_KEY_DOWN][0] = DEFAULT_GB_KEY_DOWN_0;
  (*controls)[GB_KEY_DOWN][1] = DEFAULT_GB_KEY_DOWN_1;
  (*controls)[GB_KEY_LEFT][0] = DEFAULT_GB_KEY_LEFT_0;
  (*controls)[GB_KEY_LEFT][1] = DEFAULT_GB_KEY_LEFT_1;
  (*controls)[GB_KEY_RIGHT][0] = DEFAULT_GB_KEY_RIGHT_0;
  (*controls)[GB_KEY_RIGHT][1] = DEFAULT_GB_KEY_RIGHT_1;
}

uint8_t process_controls_ini(char *ini_string, uint32_t len, emu_controls *controls)
{
  ini_file file;
  ini_parse(ini_string, len, &file);

  ini_section *section = find_section(&file, CONTROLS_INI_SECTION_NAME);

  if (!section)
  {
    free_ini_file(&file);
    return 1;
  }

  for (uint8_t i = 0; i < GB_KEY_COUNT; i++) 
  {
    char a = '0' + i;
    char key_name[4] = { a, CONTROLS_INI_KEY_SEPERATOR, '0', '\0' };

    // Find the key for the first int of the controls array
    ini_key *key_0 = find_key(section, key_name);
    
    if (!key_0)
    {
      free_ini_file(&file);
      return 1;
    }

    (*controls)[i][0] = key_0->value_int;

    key_name[2]++;

    // Find the key for the second int of the controls array
    ini_key *key_1 = find_key(section, key_name);

    if (!key_1)
    {
      free_ini_file(&file);
      return 1;
    }
    
    (*controls)[i][1] = key_1->value_int;
  }

  free_ini_file(&file);
  
  return 0;
}

char *create_controls_ini(emu_controls *controls, char *ini_string, uint32_t len)
{
  ini_file file;
  file.section_count = 0;
  file.sections_size = 0;
  
  ini_section *controls_section = add_section(&file, CONTROLS_INI_SECTION_NAME);
  
  if (!controls_section)
  {
    free_ini_file(&file);
    return nullptr;
  }

  for (uint8_t i = 0; i < GB_KEY_COUNT; i++) 
  {
    char a = '0' + i;
    char key_name[] = { a, CONTROLS_INI_KEY_SEPERATOR, '0', '\0' };
  
    if (!add_key(
      controls_section, 
      key_name,
      INI_TYPE_INT,
      (*controls)[i][0]
    ))
    {
      free_ini_file(&file);
      return nullptr;
    }

    key_name[2]++;
  
    if (!add_key(
      controls_section, 
      key_name,
      INI_TYPE_INT,
      (*controls)[i][1]
    ))
    {
      free_ini_file(&file);
      return nullptr;
    }
  }

  ini_write(&file, ini_string, len);
  free_ini_file(&file);

  return ini_string;
}

uint8_t load_controls(struct gb_s *gb)
{
  emu_controls *controls = &(((emu_preferences *)(gb->direct.priv))->controls);
  uint32_t size;
  char *ini_string;

  set_controls_defaults(controls);

  if (read_mcs(MCS_DIRECTORY, CONTROLS_INI_VAR_NAME, (void **)&ini_string, &size) == 0)
  {
    if (process_controls_ini(ini_string, size, controls) == 0) 
    {
      return 0;
    }
  }

  // Restore defaults when read failed
  set_controls_defaults(controls);
  return 1;
}

uint8_t save_controls(struct gb_s *gb)
{
  emu_controls *controls = &(((emu_preferences *)(gb->direct.priv))->controls);
  char ini_string[INI_MAX_CONTENT_LEN];
  
  create_controls_ini(controls, ini_string, sizeof(ini_string));

  uint32_t size = align_val(strlen(ini_string), 4);

  return write_mcs(MCS_DIRECTORY, CONTROLS_INI_VAR_NAME, ini_string, size);
}
