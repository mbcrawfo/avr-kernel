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
 * \brief Inline function definitions for the kernel.
 * Functions which should generally compile down to a handful of assembly 
 * instructions are inlined.
 * 
 * \see kernel_interface
 */

#ifndef KERNEL_INL_H_
#define KERNEL_INL_H_

#include <avr/pgmspace.h>

void kn_replace_self(thread_ptr entry_point, const bool suspended, void* arg)
{
  extern thread_id kn_cur_thread;
  KERNEL_ASSERT(entry_point != NULL);
  kn_create_thread(kn_cur_thread, entry_point, suspended, arg);
}

thread_id kn_current_thread()
{
  extern thread_id kn_cur_thread;
  return kn_cur_thread;
}

void kn_disable_self()
{
  extern thread_id kn_cur_thread_mask;
  extern uint8_t kn_disabled_threads;
  extern void kn_scheduler();
  
  kn_disabled_threads |= kn_cur_thread_mask;
  kn_scheduler();
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
  KERNEL_ASSERT(bit_num < 8);
  return pgm_read_byte(&kn_bitmasks[bit_num]);
}

#endif