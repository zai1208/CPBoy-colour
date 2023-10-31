/**
 * MIT License
 *
 * Copyright (c) 2023 Sidney Krombholz 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <string.h>
#include <appdef.hpp>
#include <sdk/calc/calc.hpp>
#include <sdk/os/mcs.hpp>
#include "core/emulator.h"
#include "core/error.h"
#include "cas/cpu/power.h"
#include "cas/cpu/dmac.h"

APP_NAME("CPBoy")
APP_DESCRIPTION("A Gameboy (DMG) emulator. Forked from PeanutGB by deltabeard.")
APP_AUTHOR("diddyholz")
APP_VERSION(CPBOY_VERSION)

gb_s main_gb;
emu_preferences main_preferences;

void setup_cas_cpu();
void restore_cas_cpu();

extern "C" 
int32_t main() 
{
  calcInit();
  setup_cas_cpu();

  // Create main folder for mcs vars
  MCS_CreateFolder("CPBoy", nullptr);

  if (run_emulator(&main_gb, &main_preferences) != 0) 
  {
    // Error handling
    error_crash_alert(get_error_string(errno));
  }

  restore_cas_cpu(); 
  calcEnd();
  return 0;
}

void setup_cas_cpu()
{
  // Enable DMA Controller
  POWER_MSTPCR0->raw &= ~(1 << 21);
  DMAC_DMAOR->raw = 0;
  DMAC_DMAOR->DME = 1;
}

void restore_cas_cpu() 
{
  // Disable DMA Controller
  DMAC_DMAOR->DME = 0;
  POWER_MSTPCR0->raw |= (1 << 21);
}
