#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "macros.h"

#define write_mcs(dir, name, buf, size) _write_mcs(dir, name, buf, size, __FILE__, __LINE__)

#define read_mcs(dir, name, buf, size) _read_mcs(dir, name, buf, size, __FILE__, __LINE__)

/**
 * Writes a file and does error handling. The file will be created 
 * if it does not exist
 * This should be used whenever a file is written.
 *   
 * @param file  The filename to write to
 * @param buf   The buffer to be written
 * @param len   The size of the buffer
 * 
 * @return Returns 0 on success else an error occured
*/
#define write_file(file, buf, len) _write_file(file, buf, len, __FILE__, __LINE__)

/**
 * Read a file and does error handling. 
 * This should be used whenever a file is read.
 *   
 * @param file  The filename to be read
 * @param buf   The buffer in which to read
 * @param len   The size of the buffer
 * 
 * @return Returns 0 on success else an error occured
*/
#define read_file(file, buf, len) _read_file(file, buf, len, __FILE__, __LINE__)

/**
 * Read a file and does error handling. 
 * This should be used whenever a file is read.
 *   
 * @param file  The file to be deleted
 * 
 * @return Returns 0 on success else an error occured
*/
#define delete_file(file) _delete_file(file, __FILE__, __LINE__)

/**
 * Find files matching the given path
 *   
 * @param path  The path to be searched (Can contain wildcards)
 * @param buf   The buffer in which the found files should be written to
 * @param len   The maximum amount of files to find (The size of your buffer)
 * 
 * @return Returns the amount of files found
*/
uint8_t find_files(const char *path, char (*buf)[MAX_FILENAME_LEN], uint8_t max);

uint8_t _write_mcs(const char *dir, const char *name, void *buf, size_t len, 
  const char *err_file, uint32_t err_line);
uint8_t _read_mcs(const char *dir, const char *name, void **buf, uint32_t *len, 
  const char *err_file, uint32_t err_line);
uint8_t _write_file(const char *file, void *buf, size_t len, const char *err_file, uint32_t err_line);
uint8_t _read_file(const char *file, void *buf, size_t len, const char *err_file, uint32_t err_line);
uint8_t _delete_file(const char *file, const char *err_file, uint32_t err_line);
