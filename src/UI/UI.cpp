#include <stdint.h>
#include <SDK/calc/calc.hpp>
#include "UI.hpp"

#define FONTBASE 0x8062F4C8
#define DISPLAY_WIDTH		320
#define DISPLAY_HEIGHT	528

void word_to_string(uint16_t word, char* string);

char hex_chars[17] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F' };

void print_hex_word(uint16_t word, uint16_t x, uint16_t y, uint8_t size,
	uint16_t foreground, uint16_t background, uint8_t enable_transparency)
{
	char string[7];

	uint32_t counter = 0;

	word_to_string(word, string);

	while (string[counter] != 0 && (x + (counter * DEBUG_CHAR_WIDTH)) < DISPLAY_WIDTH)
	{
		print_char(string[counter], x + (counter * ((DEBUG_CHAR_WIDTH - 1) * (size + 1))), y, 
			size, foreground, background, enable_transparency);  
		counter++; 
	}    
}

void word_to_string(uint16_t word, char* string)
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

void print_string(char *string, uint16_t x, uint16_t y, uint8_t size,
	uint16_t foreground, uint16_t background, uint8_t enable_transparency)
{
	uint32_t counter = 0;

	while (string[counter] != 0 && (x + (counter * (DEBUG_CHAR_WIDTH - 2) * (size + 1))) < DISPLAY_WIDTH)
	{
		print_char(string[counter], x + (counter * ((DEBUG_CHAR_WIDTH - 2) * (size + 1))), y, 
			size, foreground, background, enable_transparency);  
		counter++; 
	}    
}

void print_char(char character, uint16_t x, uint16_t y, uint8_t size,
	uint16_t foreground, uint16_t background, uint8_t enable_transparency)
{
	uint8_t charIndex = character - ' ';
	uint16_t tempXPos = x;
	uint16_t tempYPos = y;

	size = (size != 0);

	size++;

	// fill background with background color if it is not black
	if(background || !enable_transparency)
	{
		for(uint8_t y = 0; y < ((DEBUG_CHAR_HEIGHT + 2) * size); y++)
		{
			tempXPos = x;

			for(uint8_t x = 0; x < (DEBUG_CHAR_WIDTH * size); x++)
			{
				if(tempYPos >= DISPLAY_HEIGHT || tempXPos >= DISPLAY_WIDTH)
					continue;

				vram[(tempYPos * DISPLAY_WIDTH) + tempXPos] = background;
				tempXPos++;
			}

			tempYPos++;
		}
	}

	// now draw the character
	uint16_t *pixel = (uint16_t *)(FONTBASE + (0xC0 * charIndex));

	tempXPos = x;
	tempYPos = y;

	tempYPos++;

	for(uint8_t y = 0; y < DEBUG_CHAR_HEIGHT; y++)
	{
		tempXPos = x;

		for(uint8_t x = 0; x < DEBUG_CHAR_WIDTH; x++)
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
