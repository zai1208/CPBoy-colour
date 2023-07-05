#pragma once

#include <stdint.h>
#include "macros.h"

#define INI_MAX_SECTION_LEN 20
#define INI_MAX_KEY_LEN     20
#define INI_MAX_VALUE_LEN   MAX_FILENAME_LEN
#define INI_MAX_CONTENT_LEN 1024

#define INI_TYPE_INT    0
#define INI_TYPE_STRING 1


struct ini_key
{
  char name[INI_MAX_KEY_LEN];
  char value_str[INI_MAX_VALUE_LEN];
  uint32_t value_int;
  uint8_t value_type;
};

struct ini_section
{
  char name[INI_MAX_SECTION_LEN];
  ini_key *keys;
  uint32_t key_count;
  uint32_t keys_size;
};

struct ini_file
{
  ini_section *sections;
  uint32_t section_count;
  uint32_t sections_size;
};


// TODO: MUST BE FREED
// ini_file *ini_parse(const char *ini_string, uint32_t len, ini_file *file);
ini_file *ini_parse(const char *ini_string, uint32_t len, ini_file *file);

char *ini_write(ini_file *file, char *ini_string, uint32_t len);

ini_section *find_section(const ini_file *file, const char *name);

ini_key *find_key(const ini_section *section, const char *name);

void free_ini_file(ini_file *file);

ini_key *add_key(ini_section *section, const char *name, uint8_t value_type = INI_TYPE_INT, 
  uint32_t value_int = 0, const char *value_str = nullptr);

ini_section *add_section(ini_file *file, const char *name);
