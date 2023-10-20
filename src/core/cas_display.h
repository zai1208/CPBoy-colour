#pragma once

#include <stdint.h>

#define SERIAL_SCREEN_DATA_REGISTER 0xB4000000

inline void prepare_lcd() 
{
  ((void(*)(int, int, int, int))0x80038068)(0, CAS_LCD_WIDTH - 1, 0, (LCD_HEIGHT * 2) - 1);
  ((void(*)(int))0x80038040)(0x2c);
}
