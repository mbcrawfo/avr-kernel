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

#define TMP_REG r0
#define ZERO_REG r1

// external symbols from kernel.c
.extern kn_stack_base // program memory
.extern kn_canary_loc // program memory
.extern kn_cur_thread
.extern kn_cur_thread_mask
.extern kn_disabled_threads
.extern kn_suspended_threads
.extern kn_sleeping_threads
.extern kn_stack

  .section .text

/******************************************************************************
 * Global functions
 *****************************************************************************/

 // bool kn_create_thread(const thread_id t_id, thread_ptr entry_point, 
 //   const bool suspended, void* arg)
 // see documentation in kernel.h
 // wraps kn_create_thread_impl (see kernel.c)
 // If the calling thread is replacing itself, sets up the stack so that stack 
 // corruption is avoided
.global kn_create_thread
kn_create_thread:
  // r24, r22/23, r20, r18/19 hold the params
  // validate the thread id
  cpi r24, MAX_THREADS
  brsh .bad_tid
  // see if we are replacing the current thread
  lds r26, kn_cur_thread
  cp r26, r24
  brne .call_impl
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
  in TMP_REG, SREG
  cli
  out SPH, r26
  out SPL, r27
  out SREG, TMP_REG
  // end atomic block
.call_impl:
  call kn_create_thread_impl
  ret
.bad_tid:
  // return false
  clr r24
  ret

// void kn_yield()
// see documentation in kernel.h
// note that kn_yield falls through to the scheduler and does not return itself
.global kn_yield
kn_yield:
  // save the thread state
  push r2
  push r3
  push r4
  push r5
  push r6
  push r7
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  push r16
  push r17
  push r28
  push r29  
  // get the current thread id and make it a pointer offset
  lds r24, kn_cur_thread
  lsl r24
  // check the stack canary
#ifdef USE_STACK_CANARY
  // base canary pointer in Z
  ldi r30, lo8(kn_canary_loc)
  ldi r31, hi8(kn_canary_loc)
  // add the offset for this thread
  add r30, r24
  adc r31, ZERO_REG
  // and load the canary pointer in X
  lpm r26, Z+
  lpm r27, Z
  // load and compare the canary value
  ld r25, X
  cpi r25, STACK_CANARY
  breq .save_stack
  //TODO: error handling
#endif
.save_stack:
  // stack array pointer in X
  ldi r26, lo8(kn_stack)
  ldi r27, hi8(kn_stack)
  // add the offset for this thread
  add r26, r24
  adc r27, ZERO_REG
  // get the hardware stack pointer and save it using X
  cli
  in r24, SPL
  in r25, SPH
  st X+, r24
  st X, r25
  // interrupts stay off for the scheduler

// void kn_scheduler()
// see documentation in kernel.c
.global kn_scheduler
kn_scheduler:
  // thread id in r24, mask in r25
  lds r24, kn_cur_thread
  lds r25, kn_cur_thread_mask
  // keep a copy of the id
  mov r23, r24
.scheduler_start:
  cli
  // refresh the status masks, or them into a single mask
  lds r26, kn_disabled_threads
  lds r27, kn_suspended_threads
  lds r28, kn_sleeping_threads
  or r26, r27
  or r26, r28
.scheduler_loop:
  // shift to the next thread
  inc r24
  lsl r25
  // check if the mask has gone past the last possible thread
  cpi r24, MAX_THREADS
  brne .scheduler_test
  // if so, reset
  ldi r24, 0
  ldi r25, 0x01
.scheduler_test:
  // if none of the status bits are set this thread is ready to run
  mov r27, r26
  and r27, r25  
  breq .restore_thread
  // see if we are back to the starting id
  cp r24, r23
  brne .scheduler_loop
  // if so, no threads are ready
  // enable sleep
  in r26, SMCR
  sbr r26, SE
  out SMCR, r26
  // sleep and wait for an interrupt
  sei
  sleep
  // disable sleep
  in r26, SMCR
  cbr r26, SE
  out SMCR, r26
  // restart the scheduler
  rjmp .scheduler_start
.restore_thread:
  sei
  // save the thread id and mask
  sts kn_cur_thread, r24
  sts kn_cur_thread_mask, r25
  // stack array pointer in X
  ldi r26, lo8(kn_stack)
  ldi r27, hi8(kn_stack)
  // make the thread id a pointer offset and add it to X
  lsl r24
  add r26, r24
  adc r27, ZERO_REG
  // load the new thread's stack pointer 
  ld r24, X+
  ld r25, X
  // write it to hardware
  cli
  out SPL, r24
  out SPH, r25
  sei
  // restore the thread state
  pop r29
  pop r28
  pop r17
  pop r16
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop r7
  pop r6
  pop r5
  pop r4
  pop r3
  pop r2
  ret

// void kn_thread_bootstrap()
// see documentation in kernel.c
.global kn_thread_bootstrap
kn_thread_bootstrap:
  pop r24
  pop r23
  pop r22    
  ret