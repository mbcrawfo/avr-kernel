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

// Doxygen's parser doesn't play well with assembly, so this file isn't in the 
// main documentation.

#define __SFR_OFFSET 0

#include "config.h"
#include "stacks.h"
#include <avr/io.h>

// external symbols from kernel.c
.extern kn_stack_base // program memory
.extern kn_canary_loc // program memory
.extern kn_cur_thread
.extern kn_cur_thread_mask
.extern kn_disabled_threads
.extern kn_suspended_threads
.extern kn_delayed_threads
.extern kn_stack

  .section .text

/******************************************************************************
 * Global functions
 *****************************************************************************/

 // bool kn_create_thread(const thread_id t_id, thread_ptr entry_point, 
 //   const bool suspended, void* arg)
 // see documentation in kernel.h
 // wraps kn_create_thread_impl (see kernel.c), calling it in kernel mode
.global kn_create_thread
kn_create_thread:
  // r24, r22/23, r20, r18/19 hold the params
  // validate the thread id
  cpi r24, MAX_THREADS
  brsh .knct_bad_tid
  // see if we are replacing the current thread
  lds r26, kn_cur_thread
  cp r26, r24
  brne .knct_call_impl
  // if yes, load the stack base
  ldi r30, pm_hi8(kn_stack_base)
  ldi r31, pm_lo8(kn_stack_base)
  add r31, r24
  adc r30, r1
  lpm r26, Z+
  lpm r27, Z
  // add space for the new thread usage
  adiw r26, INITIAL_STACK_USAGE
  // set the stack pointer (atomic block restore state)
  in r0, SREG
  cli
  out SPH, r26
  out SPL, r27
  out SREG, r0
  // end atomic block
.knct_call_impl:
  call kn_create_thread_impl
  ret
.knct_bad_tid:
  // return false
  clr r24
  ret


// void kn_thread_bootstrap()
// see documentation in kernel.c
.global kn_thread_bootstrap
kn_thread_bootstrap:
  pop r24
  pop r22
  pop r23  
  ret