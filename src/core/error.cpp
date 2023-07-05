#include "error.h"

#include <string.h>
#include <stdlib.h>
#include "../emu_ui/components.h"
#include "../emu_ui/colors.h"
#include "../helpers/functions.h"

#define ERROR_ALERT_MSG_MAX_LEN 400

const char *error_messages[] = {
  ERROR_MSG_EMALLOC,
  ERROR_MSG_EFOPEN,
  ERROR_MSG_EFREAD,
  ERROR_MSG_EFWRITE,
  ERROR_MSG_EFCLOSE,
  ERROR_MSG_EMKDIR,
  ERROR_MSG_EEMUCARTRIDGE,
  ERROR_MSG_EEMUCHECKSUM,
  ERROR_MSG_EEMUGEN,
  ERROR_MSG_ESTRBUFEMPTY,
};

uint8_t errno;

char error_file[ERROR_MAX_FILE_LEN];
char error_info[ERROR_MAX_INFO_LEN];

uint32_t error_line;

void _set_error(uint8_t error, const char *file, uint32_t line, const char *info)
{
  errno = error;
  
  strlcpy(error_file, file, ERROR_MAX_FILE_LEN - 1);
  strlcpy(error_info, info, ERROR_MAX_INFO_LEN - 1);

  error_line = line;

  // Make sure string are null terminated
  error_file[ERROR_MAX_FILE_LEN - 1] = '\0';
  error_info[ERROR_MAX_INFO_LEN - 1] = '\0';
}

const char *get_error_string(uint8_t error)
{
  static char error_string[100];

  strlcpy(error_string, error_messages[error], sizeof(error_string));

  return error_string;
}

void error_crash_alert(const char *error)
{
  char error_msg[ERROR_ALERT_MSG_MAX_LEN];
  char tmp[11];

  strlcpy(error_msg, error, sizeof(error_msg));

  if (error_file[0] != '\0')
  {
    // Print error info if it exists
    if (error_info[0] != '\0')
    {
      strlcat(error_msg, " (", sizeof(error_msg));
      strlcat(error_msg, error_info, sizeof(error_msg));
      strlcat(error_msg, ")", sizeof(error_msg));
    }

    strlcat(error_msg, "\n(", sizeof(error_msg));
    strlcat(error_msg, error_file, sizeof(error_msg));
    strlcat(error_msg, ":", sizeof(error_msg));
    strlcat(error_msg, itoa(error_line, tmp, 10), sizeof(error_msg));
    strlcat(error_msg, ")\n\n", sizeof(error_msg));
  }

  strlcat(error_msg, ERROR_MSG_GEN_EMULATOR_QUIT, sizeof(error_msg));

  error_gen_alert(error_msg);
}

void error_gen_alert(const char *error)
{
  ok_alert("ERROR!", nullptr, error, COLOR_DANGER, COLOR_BLACK, COLOR_DANGER);
}