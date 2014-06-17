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
 * \brief Contains most of the kernel implementation details.
 * \see kernel_implementation
 */

#include "kernel.h"
#include "config.h"
#include "stacks.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

/**
 * \defgroup kernel_implementation Kernel Implementation
 * \brief Items that are used internally within the kernel.
 * 
 * @{
 */

/******************************************************************************
 * Local stack info
 *****************************************************************************/

/**
 * Contains pointers to the base of each stack for easier run time access.
 */
const uint8_t* const kn_stack_base[MAX_THREADS] PROGMEM = {
  THREAD0_STACK_BASE
  #if MAX_THREADS >= 2
    , THREAD1_STACK_BASE
  #endif
  #if MAX_THREADS >= 3
    , THREAD2_STACK_BASE
  #endif
  #if MAX_THREADS >= 4
    , THREAD3_STACK_BASE
  #endif
  #if MAX_THREADS >= 5
    , THREAD4_STACK_BASE
  #endif
  #if MAX_THREADS >= 6
    , THREAD5_STACK_BASE
  #endif
  #if MAX_THREADS >= 7
    , THREAD6_STACK_BASE
  #endif
  #if MAX_THREADS == 8
    , THREAD7_STACK_BASE
  #endif
};

#ifdef USE_STACK_CANARY
  /**
   * Contains pointers to each stack's canary location for easier run time 
   * access.
   */
  const uint8_t* const kn_canary_loc[MAX_THREADS] PROGMEM = {
    THREAD0_CANARY_LOC
    #if MAX_THREADS >= 2
    , THREAD1_CANARY_LOC
    #endif
    #if MAX_THREADS >= 3
    , THREAD2_CANARY_LOC
    #endif
    #if MAX_THREADS >= 4
    , THREAD3_CANARY_LOC
    #endif
    #if MAX_THREADS >= 5
    , THREAD4_CANARY_LOC
    #endif
    #if MAX_THREADS >= 6
    , THREAD5_CANARY_LOC
    #endif
    #if MAX_THREADS >= 7
    , THREAD6_CANARY_LOC
    #endif
    #if MAX_THREADS == 8
    , THREAD7_CANARY_LOC
    #endif
  };
#endif

/******************************************************************************
 * Kernel state variables
 *****************************************************************************/

/** Holds the id of the currently executing thread. */
thread_id kn_cur_thread;

/** Holds the mask of the currently executing thread. */
uint8_t kn_cur_thread_mask;

/** Tracks threads that are inactive. */
uint8_t kn_disabled_threads;

/** Tracks threads that have their execution suspended. */
uint8_t kn_suspended_threads;

/** Tracks threads that are delayed waiting for a timer. */
volatile uint8_t kn_delayed_threads;

/** Holds the saved stack locations for each thread. */
uint8_t* kn_stack[MAX_THREADS];

/** Tracks the delay times for each thread. */
static volatile uint16_t kn_delay_counter[MAX_THREADS];
  
/** Counts the total system uptime, in milliseconds. */
static volatile uint32_t kn_system_counter;

/******************************************************************************
 * External assembly functions
 *****************************************************************************/

/**
 * Pops a thread's parameters off of the stack and into the correct registers, 
 * then jumps to the new thread.
 * 
 * \see thread_ptr
 * \see kn_create_thread
 */
extern void kn_thread_bootstrap();

/**
 * Enters the scheduler without saving the state of the calling thread. Does 
 * not return. Should only be used when a thread has been disabled or it is 
 * being replaced.
 */
extern void kn_scheduler();

/******************************************************************************
 * Local function declarations
 *****************************************************************************/

/**
 * See \ref kn_create_thread for the behavior of this function.  
 * \ref kn_create_thread is a wrapper implemented that sets up the stack to 
 * avoid stack corruption when a thread replaces itself.
 * 
 * \warning As with \ref kn_create_thread, this function does not return if 
 * \c t_id is the currently active thread.
 */
extern bool kn_create_thread_impl(const thread_id t_id, thread_ptr entry_point,
  const bool suspended, void* arg);

/******************************************************************************
 * Local function definitions
 *****************************************************************************/

bool kn_create_thread_impl(const thread_id t_id, thread_ptr entry_point, 
  const bool suspended, void* arg)
{
  if (!entry_point)
  {
    return false;
  }
  
  // set the initial state of the thread's stack
  // the stack is set up so that the scheduler "returns" to the bootstrap 
  // function as if it had yielded
  // the bootstrap function then loads the thread args into the correct 
  // registers and jumps to the new thread
  kn_stack[t_id] = ((uint8_t*)pgm_read_word(&kn_stack_base[t_id])) -
    INITIAL_STACK_USAGE;  
  // 2 bytes for the entry point address
  kn_stack[t_id][25] = ((uint16_t)entry_point) & 0x00FF;
  kn_stack[t_id][24] = ((uint16_t)entry_point) >> 8;
  // 2 bytes for arg
  kn_stack[t_id][23] = ((uint16_t)arg) & 0x00FF;
  kn_stack[t_id][22] = ((uint16_t)arg) >> 8;
  // 1 byte for the thread id
  kn_stack[t_id][21] = t_id;
  // 2 bytes for the bootstrap address
  kn_stack[t_id][20] = ((uint16_t)kn_thread_bootstrap) & 0x00FF;
  kn_stack[t_id][19] = ((uint16_t)kn_thread_bootstrap) >> 8;
  // the remaining 18 bytes are popped to restore registers
  // their value doesn't actually matter they just need to be on the stack
  
  // update kernel state for the new thread
  uint8_t mask = bit_to_mask(t_id);
  kn_disabled_threads &= ~mask;
  kn_delayed_threads &= ~mask;
  kn_suspended_threads = 
    suspended ? (kn_suspended_threads | mask) : (kn_suspended_threads & ~mask);
  kn_delay_counter[t_id] = 0;
  
  if (t_id == kn_cur_thread)
  {
    kn_scheduler();
  }
  
  return true;
}

/******************************************************************************
 * External function definitions
 *****************************************************************************/

uint8_t bit_to_mask(uint8_t bit_num)
{
  static const uint8_t bitmasks[] PROGMEM = { 
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
  };
  
  return bit_num < 8 ? pgm_read_byte(&bitmasks[bit_num]) : 0;
}

/******************************************************************************
 * Definitions of automatically called functions
 *****************************************************************************/

/**
 * Provides initialization of the kernel. Is automatically called in the .init8 
 * section, just before \c main() is called. When the program enters \c main() 
 * the kernel is running with only \ref THREAD0 active. The user must enable 
 * interrupts before using any of the timing features of the kernel.
 * 
 * \warning Due to the way that the linker processes symbols, this function 
 * will only be inserted in the .init8 section if another function from this 
 * source file is used.
 * 
 * \warning If for any reason you try to manually call this function after 
 * \c main() has been called, you'll totally break your program...
 */
static void kn_init() __attribute__((naked, section(".init8"), used));
void kn_init()
{ 
  // initialize each thread's state
  for (uint8_t i = 0; i < MAX_THREADS; i++)
  {
    kn_stack[i] = (uint8_t*)pgm_read_word(&kn_stack_base[i]);
    kn_delay_counter[i] = 0;

#ifdef USE_STACK_CANARY
    uint8_t* canary = (uint8_t*)pgm_read_word(&kn_canary_loc[i]);
    *canary = STACK_CANARY;
#endif
  }
  
  // running thread becomes THREAD0
  kn_cur_thread = THREAD0;
  kn_cur_thread_mask = 0x01;
  // THREAD0 is the only enabled thread
  kn_disabled_threads = ~kn_cur_thread_mask;
  // no threads suspended
  kn_suspended_threads = 0x00;
  // no threads delayed
  kn_delayed_threads = 0x00;
  // set the stack for THREAD0
  SP = (uint16_t)kn_stack[THREAD0];
  
  // reset system counter
  kn_system_counter = 0;
  
  // sleep mode idle, sleep disabled
  SMCR = 0;
}

/**
 * @}
 */
