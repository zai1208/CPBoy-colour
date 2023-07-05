#include "cas_display.h"

#include <stdint.h>
#include <sdk/calc/calc.hpp>
#include "../helpers/macros.h"

#define SERIAL_SCREEN_DATA_REGISTER_ADDR 0xb4000000

// Some big brain custom LCD_Refresh() implementation by @DasHeiligeDoenerhuhn
void lcd_refresh_partial(int x_start, int x_end, int y_start, int y_end)
{
  ((void(*)(int, int, int, int))0x80038068)(x_start, x_end - 1, y_start, y_end - 1);
  ((void(*)(int))0x80038040)(0x2c);

  uint32_t offset = (y_start * CAS_LCD_WIDTH) + x_start;
  uint16_t* ssdr = (uint16_t*)SERIAL_SCREEN_DATA_REGISTER_ADDR;
    
  for (uint16_t y = 0; y < y_end; y++)
  {
    uint16_t* pixel = (uint16_t*)(vram + offset);

    for (uint16_t x = 0; x < x_end; x++)
    {
      *ssdr = *pixel;
      pixel++;
    }

    offset += CAS_LCD_WIDTH;
  }
}