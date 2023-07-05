#pragma once

#include <stdint.h>

#define EMALLOC         0
#define EFOPEN          1
#define EFREAD          2
#define EFWRITE         3
#define EFCLOSE         4
#define EMKDIR          5
#define EEMUCARTRIDGE   6
#define EEMUCHECKSUM    7
#define EEMUGEN         8
#define ESTRBUFEMPTY    9

#define ERROR_MSG_EMALLOC             "Failed to allocate memory"
#define ERROR_MSG_EFOPEN              "Failed to open file"
#define ERROR_MSG_EFREAD              "Failed to read file"
#define ERROR_MSG_EFWRITE             "Failed to write to file"
#define ERROR_MSG_EFCLOSE             "Failed to close file"
#define ERROR_MSG_EMKDIR              "Failed to make directory"
#define ERROR_MSG_EEMUCARTRIDGE       "Unsupported cardridge format"
#define ERROR_MSG_EEMUCHECKSUM        "ROM Checksum failure"
#define ERROR_MSG_EEMUGEN             "Unknown error on emulator context initialization"
#define ERROR_MSG_ESTRBUFEMPTY        "The string buffer ran out of space"
#define ERROR_MSG_GEN_EMULATOR_RETRY  "Please try a different ROM"
#define ERROR_MSG_GEN_EMULATOR_QUIT   "The emulator will now quit"

#define ERROR_MAX_FILE_LEN 100
#define ERROR_MAX_INFO_LEN 100

#define set_error(error) _set_error(error, __FILE__, __LINE__, "")
#define set_error_i(error, info) _set_error(error, __FILE__, __LINE__, info)

/*!
	@brief Stores information about an error
*/
extern uint8_t errno;

/*!
	@brief The file in which an error occured
*/
extern char error_file[ERROR_MAX_FILE_LEN];

/*!
	@brief The file in which an error occured
*/
extern char error_info[ERROR_MAX_INFO_LEN];

/*!
	@brief The line at which an error occured
*/
extern uint32_t error_line;

/*!
	@brief Sets error parameters when an error occured
  @param error The errno to be set
  @param file The file where the error occured
  @param line The line where the error occured
  @param info An info about the error (e.g. file name for file errors)
*/
void _set_error(uint8_t error, const char *file, uint32_t line, const char *info);

/*!
	@brief Gets a user readable string to the error message
	@param error The error message to be printed
*/
const char *get_error_string(uint8_t error);

/*!
	@brief Prints an error message and an emulator is quitting message.
         Will automatically add file and line number. 
	@param error The error message to be printed
*/
void error_crash_alert(const char *error);

/*!
	@brief Prints a generic error message
	@param error The error message to be printed
*/
void error_gen_alert(const char *error);