#pragma once

#include <stdint.h>
#include "menu/menu.h"

#define INPUT_PROC_NONE     0     
#define INPUT_PROC_EXECUTE  1  
#define INPUT_PROC_CLOSE    2

uint8_t process_input(uint8_t **selected_h_item, uint8_t *selected_v_item, 
  const uint8_t *h_item_count, uint8_t v_item_count, const menu_item *items, bool reset_v_item);

void wait_input_release();
