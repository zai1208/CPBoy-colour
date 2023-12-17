#include "bootstrap.h"

#include <stdint.h>
#include <string.h>
#include <sdk/os/mcs.hpp>
#include "cpu/cpg.h"
#include "cpu/cmt.h"
#include "cpu/dmac.h"
#include "cpu/oc_mem.h"
#include "cpu/power.h"
#include "../core/error.h"
#include "../helpers/fileio.h"

// All external binaries have to be defined here
const char *bin_files[] = {
  "il.bin",
  "y.bin"
};

void *load_addresses[] = {
  IL_MEMORY,
  Y_MEMORY_0
};

uint8_t load_bins(const char **bin_files, void **load_addresses, size_t bin_count);

uint8_t setup_cas()
{
  // Load external binaries
  if (load_bins(bin_files, load_addresses, sizeof(load_addresses) / sizeof(void *))) 
  {
    return 1;
  } 

  // Enable DMA Controller
  POWER_MSTPCR0->DMAC = 0;
  DMAC_DMAOR->raw = 0;
  DMAC_DMAOR->DME = 1;

  // Enable Timers
  POWER_MSTPCR0->TMU = 0;
  POWER_MSTPCR0->CMT = 0;

  // Create main folder for mcs vars
  MCS_CreateFolder("CPBoy", nullptr); 

  return 0;
}

void restore_cas() 
{
  // Disable DMA Controller
  DMAC_DMAOR->DME = 0;
  POWER_MSTPCR0->DMAC = 1;

  // Disable Timers
  cmt_stop();

  POWER_MSTPCR0->CMT = 1;
  POWER_MSTPCR0->TMU = 1;
  
  // Restore clock speed
  cpg_set_pll_mul(CPG_PLL_MUL_DEFAULT);
}

uint8_t load_bins(const char **bin_files, void **load_addresses, size_t bin_count) 
{
  for (size_t i = 0; i < bin_count; i++) 
  {
    char bin_path[MAX_FILENAME_LEN] = DIRECTORY_BIN "\\" ;
    strlcat(bin_path, bin_files[i], sizeof(bin_path));

    size_t bin_size;

    if (get_file_size(bin_path, &bin_size))
    {
      return 1;
    }

    if (read_file(bin_path, load_addresses[i], bin_size))
    {
      return 1;
    }
  }

  return 0;
}
