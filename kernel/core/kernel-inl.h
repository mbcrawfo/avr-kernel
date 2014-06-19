/******************************************************************************
  avr-kernel
  Copyright (C) 2014 Michael Crawford

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
******************************************************************************/

/** \file
 * \brief Contains inline function definitions for the kernel.
 * \see kernel_interface
 */

#ifndef KERNEL_INL_H_
#define KERNEL_INL_H_

#include "config.h"
#include <avr/pgmspace.h>

thread_id kn_current_thread()
{
  extern thread_id kn_cur_thread;
  return kn_cur_thread;
}

bool kn_thread_enabled(const thread_id t_id)
{
  extern uint8_t kn_disabled_threads;
  
  if (t_id < MAX_THREADS)
  {
    return (kn_disabled_threads & bit_to_mask(t_id)) == 0;
  }
  
  return false;
}

bool kn_thread_suspended(const thread_id t_id)
{
  extern uint8_t kn_disabled_threads;
  extern uint8_t kn_suspended_threads;
  
  if (t_id < MAX_THREADS)
  {
    uint8_t mask = bit_to_mask(t_id);
    return ((kn_disabled_threads & mask) == 0) && 
           ((kn_suspended_threads & mask) != 0);
  }
  
  return false;
}

bool kn_thread_sleeping(const thread_id t_id)
{
  extern uint8_t kn_disabled_threads;
  extern volatile uint8_t kn_sleeping_threads;
  
  if (t_id < MAX_THREADS)
  {
    uint8_t mask = bit_to_mask(t_id);
    return ((kn_disabled_threads & mask) == 0) &&
           ((kn_sleeping_threads & mask) != 0);
  }
  
  return false;
}

void kn_disable(const thread_id t_id)
{
  extern thread_id kn_cur_thread;
  extern uint8_t kn_disabled_threads;
  extern void kn_scheduler();
  
  if (t_id < MAX_THREADS)
  {
    kn_disabled_threads |= bit_to_mask(t_id);
    if (t_id == kn_cur_thread)
    {
      kn_scheduler();
    }
  }
}

void kn_disable_self()
{
  extern thread_id kn_cur_thread_mask;
  extern uint8_t kn_disabled_threads;
  extern void kn_scheduler();
  
  kn_disabled_threads |= kn_cur_thread_mask;
  kn_scheduler();
}

void kn_resume(const thread_id t_id)
{
  extern uint8_t kn_suspended_threads;
  if (t_id < MAX_THREADS)
  {
    kn_suspended_threads &= ~bit_to_mask(t_id);
  }
}

void kn_suspend(const thread_id t_id)
{
  extern thread_id kn_cur_thread;
  extern uint8_t kn_suspended_threads;
  
  if (t_id < MAX_THREADS)
  {
    kn_suspended_threads |= bit_to_mask(t_id);
    if (t_id == kn_cur_thread)
    {
      kn_yield();
    }
  }
}

void kn_suspend_self()
{
  extern thread_id kn_cur_thread_mask;
  extern uint8_t kn_suspended_threads;
  
  kn_suspended_threads |= kn_cur_thread_mask;
  kn_yield();
}

uint8_t bit_to_mask(uint8_t bit_num)
{
  extern const uint8_t kn_bitmasks[8] PROGMEM;
  return pgm_read_byte(&kn_bitmasks[bit_num]);
}

#endif