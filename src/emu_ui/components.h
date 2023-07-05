#pragma once

#include <stdint.h>
#include "colors.h"

#define STD_CONTENT_OFFSET     6
#define ALERT_CONTENT_OFFSET_X STD_CONTENT_OFFSET
#define ALERT_CONTENT_OFFSET_Y (STD_CONTENT_OFFSET * 2) + (DEBUG_LINE_HEIGHT * 2)

#define SLIDER_STD_TRACK_COLOR COLOR_DISABLED
#define SLIDER_HANDLE_HEIGHT 12

#define ALERT_GET_X(val) (val & 0xFFFF)
#define ALERT_GET_Y(val) (val >> 16)

void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
	uint16_t color, uint16_t border_width, uint16_t border_color);
  
void draw_slider(uint16_t x, uint16_t y, uint16_t width, uint16_t track_color,
	uint16_t handle_color, uint16_t min_value, uint16_t max_value, uint16_t value);

uint32_t draw_alert_box(const char *title, const char *subtitle, uint16_t width, 
  uint16_t height, uint16_t background, uint16_t border);

void ok_alert(const char *title, const char *subtitle, const char *text, uint16_t foreground, 
  uint16_t background, uint16_t border);