#include "effects.h"

#include <stdint.h>
#include <sdk/os/lcd.hpp>
#include <sdk/calc/calc.hpp>

#define CAS_LCD_WIDTH  320 
#define CAS_LCD_HEIGHT 528 

#define EFFECT_DARKEN  13108 

void darken_screen_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height) 
{
  uint16_t max_x = x + width;
  uint16_t max_y = y + height;

  // go through every pixel of the area and darken it
  for(uint16_t iy = y; iy < max_y; iy++)
  {
    for(uint16_t ix = x; ix < max_x; ix++)
    {
      uint16_t pixel = vram[(iy * CAS_LCD_WIDTH) + ix];

      // calculate new rgb values through fixed point arithmetic    
      uint8_t red = ((RGB565_TO_R(pixel) * EFFECT_DARKEN) >>16);
      uint8_t green = ((RGB565_TO_G(pixel) * EFFECT_DARKEN) >>16);
      uint8_t blue = ((RGB565_TO_B(pixel) * EFFECT_DARKEN) >>16);
    
      pixel = RGB_TO_RGB565(red, green, blue); 

      vram[(iy * CAS_LCD_WIDTH) + ix] = pixel;
    }
  }
}
