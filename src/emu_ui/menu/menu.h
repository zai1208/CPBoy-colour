#pragma once

#include <stdint.h>
#include "../../core/emulator.h"
#include "../../helpers/macros.h"

#define MENU_CLOSED   0
#define MENU_LOAD_NEW 1
#define MENU_CRASH    2
#define MENU_EMU_QUIT 3

#define TAB_DESCR_MAX_FILENAME_LENGTH 42

typedef struct menu_item
{
  bool disabled;                  // Disabled state of the item
  char title[MAX_FILENAME_LEN];   // The title of the menu item
  char value[MAX_FILENAME_LEN];   // The secondary value of the item (e.g. Enabled/Disabled)
  uint16_t value_color;           // The color of the value text
  int32_t (*action)(menu_item *, gb_s*); // The function that runs, when the item was clicked
} menu_item;

typedef struct
{
  char title[10];         // The title of the menu tab
  char description[100];  // The description of the menu tab
  uint8_t item_count;     // Amount of menu items
  menu_item *items;       // Array of menu items in this tab
} menu_tab;

typedef struct 
{
  uint16_t x_pos;
  uint16_t y_pos;
  uint16_t width;
  uint16_t height;
  uint16_t background;   // The background color of the menu
  uint8_t selected_tab;  // The currently selected tab
  uint8_t selected_item; // The currently selected item in the selected tab
  uint8_t tab_count;     // Amount of menu tabs
  menu_tab *tabs;        // Array of menu tabs
} menu;

/*!
	@brief Draws the pause overlay
*/
void draw_pause_overlay();

/*!
	@brief Draws the menu overlay
*/
void draw_menu_overlay();

/*!
	@brief Draws a menu
  @param menu A pointer to the menu to be drawn
*/
void draw_menu(menu *menu);


uint8_t load_menu(emu_preferences *prefs);

/*!
	@brief Switches context to the emulation menu, 
    returns when either the menu is closed or the emulator is quit
  @return Returns MENU_EMU_QUIT, MENU_CLOSED or MENU_LOAD_NEW
*/
uint8_t emulation_menu(struct gb_s *gb, bool preview_only);

void draw_load_alert(void);
