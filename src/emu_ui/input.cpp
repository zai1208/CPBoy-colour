#include "input.h"

#include <stdint.h>
#include <sdk/os/input.hpp>
#include <sdk/calc/calc.hpp>
#include "menu/menu.h"

uint32_t key_streak;  // Holds the streak amount
uint32_t streak_key1; // Holds key1 for the current streak
uint32_t streak_key2; // Holds key2 for the current streak

inline void reset_key_streak()
{
  key_streak = 0;
  streak_key1 = 0;
  streak_key2 = 0;
}

inline void set_key_streak(uint32_t key1, uint32_t key2)
{
  key_streak = 1;
  streak_key1 = key1;
  streak_key2 = key2;
}

void wait_input_release()
{
  uint32_t key1;
  uint32_t key2;

  do
  { 
    getKey(&key1, &key2);
  }
  while (key1 | key2);
  
  reset_key_streak();
}

uint8_t process_input(uint8_t **selected_h_item, uint8_t *selected_v_item, 
  const uint8_t *h_item_count, uint8_t v_item_count, const menu_item *items, bool reset_v_item) 
{
  uint32_t key1;
  uint32_t key2;

  // Check if keys are pressed 
  if (!Input_IsAnyKeyDown())
  {
    reset_key_streak();

    return INPUT_PROC_NONE;
  }

  getKey(&key1, &key2);

  // Check if keystreak should be increased
  if (key1 == streak_key1 && key2 == streak_key2)
  {
    key_streak++;
  }
  else 
  {
    // Set key streak to current button
    set_key_streak(key1, key2);
  }

  // Check if currently in key streak
  if (key_streak > 1)
  {
    // Default delay of 4 except for first button press
    uint8_t delay = (key_streak == 2)? 30 : 4;
    
    for (uint8_t i = 0; i < delay; i++)
    {
      LCD_Refresh(); // Refresh to create a delay

      // Check if key is still pressed
      getKey(&key1, &key2);

      if (key1 != streak_key1 || key2 != streak_key2)
      {
        set_key_streak(key1, key2);
        break;
      }
    }
  }
  
  // Process input
  if (testKey(key1, key2, KEY_DOWN))
  {
    uint8_t last_active = *selected_v_item;

    // Skip disabled items
    // This will cause an endless loop if every item is disabled
    do
    {
      if (items && !items[*selected_v_item].disabled)
      {
        last_active = *selected_v_item;
      }

      if (*selected_v_item != v_item_count - 1)
      {
        (*selected_v_item)++;
      }
      else
      {
        *selected_v_item = last_active;
      }
    } while (items && items[*selected_v_item].disabled);

    return INPUT_PROC_NONE;
  }

  if (testKey(key1, key2, KEY_UP))
  {
    uint8_t last_active = *selected_v_item;

    // Skip disabled items
    // This will cause an endless loop if every item is disabled
    do
    {
      if (items && !items[*selected_v_item].disabled)
      {
        last_active = *selected_v_item;
      }

      if (*selected_v_item != 0) 
      {
        (*selected_v_item)--;
      }
      else
      {
        *selected_v_item = last_active;
      }
    } while (items && items[*selected_v_item].disabled);

    return INPUT_PROC_NONE;
  }

  if (testKey(key1, key2, KEY_LEFT))
  {
    if (*(selected_h_item[*selected_v_item]) != 0)  
    {
      (*(selected_h_item[*selected_v_item]))--;

      // Select first item on tab
      if (reset_v_item)
      {
        *selected_v_item = 0;
      }
    }

    return INPUT_PROC_NONE;
  }

  if (testKey(key1, key2, KEY_RIGHT))
  {
    if (selected_h_item[*selected_v_item] && *(selected_h_item[*selected_v_item]) != (h_item_count[*selected_v_item] - 1)) 
    {
      (*(selected_h_item[*selected_v_item]))++;

      // Select first item on tab
      if (reset_v_item)
      {
        *selected_v_item = 0;
      }
    }

    return INPUT_PROC_NONE;
  }

  // Execute item action
  if (testKey(key1, key2, KEY_EXE))
  {
    return INPUT_PROC_EXECUTE;
  }

  if (testKey(key1, key2, KEY_NEGATIVE))
  {
    return INPUT_PROC_CLOSE;
  }

  return INPUT_PROC_NONE;
}
