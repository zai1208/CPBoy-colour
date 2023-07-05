#pragma once

#include <stdint.h>

/// @brief The width of a character in pixels
#define DEBUG_CHAR_WIDTH 	8

/// @brief The height of a character in pixels
#define DEBUG_CHAR_HEIGHT	12

/// @brief The height of a line in pixels
#define DEBUG_LINE_HEIGHT	14

/*!
	@brief Converts a 16-bit unsigned int to a hex string
	@param string The string buffer
	@param word The 16-bit unsigned int to be converted
*/
void word_to_string(char *string, uint16_t word);

/*!
	@brief Print a character at an absolute position on the screen
	@param character The character to print
	@param x The x-position where the character should be printed in pixels
	@param y The y-position where the character should be printed in pixels
	@param size Pass 0 to print in normal size and 1 to print double sized
	@param foreground The color in which the text should be printed
	@param background The background of the text
	@param enable_transparency Pass 0 to disable the use of black (0x0000) background as transparent
*/
void print_char(char character, uint16_t x, uint16_t y, uint8_t size,
	uint16_t foreground, uint16_t background, bool enable_transparency);

/*!
	@brief Print a string at an absolute position on the screen with automatic newline
	@param string A pointer to the string to be printed
	@param x The x-position where the character should be printed in pixels
	@param y The y-position where the character should be printed in pixels
	@param size Pass 0 to print in normal size and 1 to print double sized
	@param foreground The color in which the text should be printed
	@param background The background of the text
	@param enable_transparency Pass 0 to disable the use of black (0x0000) background as transparent
*/
void print_string(const char *string, uint16_t x, uint16_t y, uint8_t size,
	uint16_t foreground, uint16_t background, bool enable_transparency);

/*!
	@brief Prints a string horizontally centered at an absolute position on the screen with automatic newline
	@param string A pointer to the string to be printed
	@param x The x-position where the character should be printed in pixels
	@param y The y-position where the character should be printed in pixels
	@param width The width of the textarea in which the string should be printed
	@param size Pass 0 to print in normal size and 1 to print double sized
	@param foreground The color in which the text should be printed
	@param background The background of the text
	@param enable_transparency Pass 0 to disable the use of black (0x0000) background as transparent
*/
void print_string_centered(const char *string, uint16_t x, uint16_t y, uint16_t width,
  uint8_t size, uint16_t foreground, uint16_t background, bool enable_transparency);

/*!
	@brief Print a string at an absolute position on the screen with automatic newline (Max n chars)
	@param string A pointer to the string to be printed
	@param n The max number of characters to be printed (pass 0 to ignore)
	@param x The x-position where the character should be printed in pixels
	@param y The y-position where the character should be printed in pixels
	@param x_max The x-position at which a newline should be printed
	@param size Pass 0 to print in normal size and 1 to print double sized
	@param foreground The color in which the text should be printed
	@param background The background of the text
	@param enable_transparency Pass 0 to disable the use of black (0x0000) background as transparent
  @return Returns the number of characters printed
*/
uint16_t print_n_string(const char *string, uint16_t n, uint16_t x, uint16_t y, uint16_t x_max, 
  uint8_t size, uint16_t foreground, uint16_t background, bool enable_transparency);
