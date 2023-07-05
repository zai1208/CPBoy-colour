#include "font.h"

#include <string.h>
#include <stdint.h>
#include <sdk/calc/calc.hpp>
#include "components.h"

#define FONTBASE 0x8062F4C8
#define DISPLAY_WIDTH		320
#define DISPLAY_HEIGHT	528

char hex_chars[17] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F' };

void word_to_string(char *string, uint16_t word)
{
	string[0] = '0';
	string[1] = 'x';
	string[6] = 0;

	for(uint8_t x = 0; x < 4; x++)
	{
		string[x] = hex_chars[word % 16];
		word /= 16;
	}
}

uint16_t print_n_string(const char *string, uint16_t n, uint16_t x, uint16_t y, uint16_t x_max, 
  uint8_t size, uint16_t foreground, uint16_t background, bool enable_transparency)
{
  uint16_t char_counter = 0;
  uint16_t cur_line_counter = 0;

  for (const char *p = string; *p != '\0'; p++, char_counter++) 
  {
    if (n != 0 && char_counter == n) return char_counter;

    uint16_t x_pos = x + (cur_line_counter * ((DEBUG_CHAR_WIDTH - 2) * (size + 1)));
    
    // Check if newline
    if (x_pos > x_max || *p == '\n') 
    {
      y += DEBUG_LINE_HEIGHT;
      cur_line_counter = 0;
      continue;
    }

		print_char(
      *p, 
      x_pos, 
      y, 
			size, 
      foreground, 
      background, 
      enable_transparency
    );  

    cur_line_counter++;
  }

  return char_counter;
}

void print_string(const char *string, uint16_t x, uint16_t y, uint8_t size,
	uint16_t foreground, uint16_t background, bool enable_transparency)
{
  print_n_string(
    string,
    0,
    x,
    y,
    DISPLAY_WIDTH,
    size,
    foreground,
    background,
    enable_transparency
  );
}

void print_string_centered(const char *string, uint16_t x, uint16_t y, uint16_t width,
  uint8_t size, uint16_t foreground, uint16_t background, bool enable_transparency)
{
  const uint16_t actual_x = x + ((width - (strlen(string) * (DEBUG_CHAR_WIDTH - 2))) / 2);

  // Print background accross entire width
	if(!enable_transparency || background)
	{
    draw_rectangle(
      x,
      y,
      width,
      DEBUG_LINE_HEIGHT * size,
      background,
      0,
      0
    );
	}

  print_n_string(
    string,
    0,
    actual_x,
    y,
    DISPLAY_WIDTH,
    0,
    foreground,
    background,
    enable_transparency
  );
}

void print_char(char character, uint16_t x, uint16_t y, uint8_t size,
	uint16_t foreground, uint16_t background, bool enable_transparency)
{
	uint8_t charIndex = character - ' ';

	size = (size != 0);
	size++;

	// fill background with background color if it is not black
	if(!enable_transparency || background)
	{
    draw_rectangle(
      x,
      y,
      DEBUG_CHAR_WIDTH * size,
      (DEBUG_CHAR_HEIGHT + 2) * size,
      background,
      0,
      0
    );
	}

	// now draw the character
	uint16_t *pixel = (uint16_t *)(FONTBASE + (0xC0 * charIndex));

	uint16_t tempXPos = x;
	uint16_t tempYPos = y + 1;

	for(uint8_t iy = 0; iy < DEBUG_CHAR_HEIGHT; iy++)
	{
		tempXPos = x;

		for(uint8_t ix = 0; ix < DEBUG_CHAR_WIDTH; ix++)
		{
			if(*pixel == 0)
			{
				if(tempYPos >= DISPLAY_HEIGHT || tempXPos >= DISPLAY_WIDTH)
					continue;
					
				vram[(tempYPos * DISPLAY_WIDTH) + tempXPos] = foreground;

				if(size == 2)
				{
					vram[(tempYPos * DISPLAY_WIDTH) + tempXPos + 1] = foreground;
					vram[((tempYPos + 1) * DISPLAY_WIDTH) + tempXPos] = foreground;
					vram[((tempYPos + 1) * DISPLAY_WIDTH) + tempXPos + 1] = foreground;
				}
			}
			
			pixel++;   
			tempXPos += size;
		}

		tempYPos += size;
	}
}
