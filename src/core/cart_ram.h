#pragma once

#include <stdlib.h>
#include <stdint.h>

uint8_t load_cart_ram(struct gb_s *gb, size_t len);

uint8_t save_cart_ram(struct gb_s *gb, size_t len);
