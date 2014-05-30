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
.extern kn_canary_loc
.extern kn_cur_thread
.extern kn_cur_thread_mask
.extern kn_disabled_threads
.extern kn_suspended_threads
.extern kn_delayed_threads
.extern kn_stack

// Holds the status of kernel mode
#define KERNEL_STATUS_REG GPIOR0
// Set when operating in kernel mode
#define KERNEL_MODE_BIT 0
// Set when an interrupt occurs in kernel mode, signalling to stay in kernel 
// mode following the interrupt
#define KERNEL_NESTED_BIT 1

// Holds the lo8 of the user thread stack while in kernel mode
#define USER_STACK_LO8 GPIOR1
// Holds the hi8 of the user thread stack while in kernel mode
#define USER_STACK_HI8 GPIOR2

  .section .text

/******************************************************************************
 * Local functions
 *****************************************************************************/

// Moves into kernel mode.  While in kernel mode the user stack pointer is 
// saved in GPIOR1/GPIOR2 and the kernel stack is used.
//
// Takes a single parameter in the Z register: the address the function will 
// return to.
// R26 is used by the function and should be preserved before calling it.
//
// This function must be called with jmp/rjmp.
// Do not use the call intruction or call it from C.
// Interrupts must be disabled before calling this function.
// Trying to enter kernel mode when both KERNEL_MODE_BIT and KERNEL_NESTED_BIT 
// are already set will probably result in bad things happening.
enter_kernel_mode:
  // if called when in kernel mode, just set the nested bit
  sbic KERNEL_STATUS_REG, KERNEL_MODE_BIT
  rjmp .ekm_set_nested
  // otherwise set kernel mode
  sbi KERNEL_STATUS_REG, KERNEL_MODE_BIT
  // save the stack pointer
  in r26, SPL
  out USER_STACK_LO8, r26
  in r26, SPH
  out USER_STACK_HI8, r26
  // and swap in the kernel stack
  ldi r26, lo8(KERNEL_STACK_BASE)
  out SPL, r26
  ldi r26, hi8(KERNEL_STACK_BASE)
  out SPH, r26
  ijmp
.ekm_set_nested:
  sbi KERNEL_STATUS_REG, KERNEL_NESTED_BIT
  ijmp

// Leaves kernel mode, restoring the user stack.  If KERNEL_NESTED_BIT is set, 
// that flag is cleared and the stack is not changed.
//
// Takes a single parameter in the Z register: the address the function will 
// return to.
// R26 is used by the function and should be preserved before calling it.
//
// This function must be called with jmp/rjmp.
// Do not use the call intruction or call it from C.
// Interrupts must be disabled before calling this function.
// Calling the function when not in kernel mode will probably results in bad 
// things happening.
leave_kernel_mode:
  // if called when nested, just clear the nested bit
  sbic KERNEL_STATUS_REG, KERNEL_NESTED_BIT
  rjmp .lkm_clear_nested
  // otherwise leave kernel mode
  cbi KERNEL_STATUS_REG, KERNEL_MODE_BIT
  // restore user stack (no need to save kernel stack)
  in r26, USER_STACK_LO8
  out SPL, r26
  in r26, USER_STACK_HI8
  out SPH, r26
  ijmp
.lkm_clear_nested:
  cbi KERNEL_STATUS_REG, KERNEL_NESTED_BIT
  ijmp

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
  // r26 and Z are caller save
  ldi r30, pm_lo8(.knct_enter_ret)
  ldi r31, pm_hi8(.knct_enter_ret)
  jmp enter_kernel_mode
.knct_enter_ret:
  call kn_create_thread_impl
  // r24 holds return value
  ldi r30, pm_lo8(.knct_leave_ret)
  ldi r31, pm_hi8(.knct_leave_ret)
  jmp leave_kernel_mode
.knct_leave_ret:
  ret

// void kn_thread_bootstrap()
// see documentation in kernel.c
.global kn_thread_bootstrap
kn_thread_bootstrap:
  pop r24
  pop r22
  pop r23  
  ret