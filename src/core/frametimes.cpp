#include "frametimes.h"

#include "peanut_gb_header.h"
#include "preferences.h"
#include "../cas/cpu/cmt.h"

#define FRAME_TARGET 60

void frametime_counter_set(struct gb_s *gb)
{
  emu_preferences *pref = (emu_preferences *)gb->direct.priv;
  uint8_t frameskip = (pref->config.frameskip_enabled)?
    pref->config.frameskip_amount + 1 : 1;

  cmt_set((CMT_TICKS_PER_SEC / FRAME_TARGET) * frameskip, MODE_ONE_SHOT, REQUEST_DISABLE);
}

void frametime_counter_start()
{
  cmt_start();
}

void frametime_counter_wait(struct gb_s *gb)
{
  if (!gb->direct.frame_drawn)
  {
    return;
  }
  
  cmt_wait();
}