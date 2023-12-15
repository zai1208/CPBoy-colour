#pragma once

#include <stdint.h>
#include "peanut_gb_header.h"

void frametime_counter_set(struct gb_s *gb);

void frametime_counter_start();

void frametime_counter_wait(struct gb_s *gb);
