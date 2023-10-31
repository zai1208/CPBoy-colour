#pragma once

#include <stdint.h>

union power_mstpcr0 
{
  struct
  {
    // TODO: Bitfields
  };
  
  uint32_t raw;
};

#define POWER_MSTPCR0 ((volatile power_mstpcr0 *)0xA4150030)
