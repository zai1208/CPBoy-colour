#pragma once

#include <stdint.h>

template <typename T>
T clamp(T val, T min, T max)
{
  T tmp = (val < min)? min : val;
  return (tmp > max)? max : tmp;
}

template <typename T>
T hash_string(const char *str, T range)
{
  T h = 0;  

  const char *p;

  for (p = str; *p != '\0'; p++) 
  {
    h = ((h << 7) ^ (h >> 25)) ^ *p;
  }

  return h % range;
}

char *itoa_leading_zeros(uint32_t val, char *str, uint8_t base, uint8_t digits);
wchar_t *char_to_wchar(wchar_t *wstr, const char *str);
char *wchar_to_char(char *str, const wchar_t *wstr);
uint32_t align_val(uint32_t val, uint32_t at);
