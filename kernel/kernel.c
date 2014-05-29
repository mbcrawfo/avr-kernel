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

/**
 * \defgroup kernel_implementation Kernel Implementation
 * \brief Items that are used internally within the kernel.
 */

#include "kernel.h"
#include "config.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

/******************************************************************************
 * General constants
 *****************************************************************************/

/**
 * The amount of space used when a stack is set up for a new thread.
 * 
 * Space requirements are: entry point address (2 bytes), thread parameters 
 * (\ref thread_id and pointer; 3 bytes), bootstrap function address (2 bytes), 
 * 18 callee save registers (18 bytes).  The callee save registers are 
 * necessary because a thread is entered by returning from the scheduler in the 
 * same manner as yielding.
 * 
 * \see stack_size
 * \ingroup kernel_interface
 */
#define INITIAL_STACK_USAGE 25

/**
 * The minimum size of each stack.
 * \see stack_size
 * \ingroup kernel_interface
 */
#define MIN_STACK_SIZE 32

/**
 * The total size of the RAM available on the MCU.
 * \ingroup kernel_implementation
 */
#define TOTAL_RAM_SIZE (RAMEND - RAMSTART)

/******************************************************************************
 * Error checking of config.h values
 *****************************************************************************/

// sanity check usable thread count
#if !defined(MAX_THREADS)
  #error "MAX_THREADS not defined"
#elif (MAX_THREADS < 1) || (MAX_THREADS > 8)
  #error "MAX_THREADS must be greater than 0 and less than 8"
#endif

// verify that canary value is defined if needed 
#if defined(USE_STACK_CANARY) && !defined(STACK_CANARY)
  #error "USE_STACK_CANARY defined but STACK_CANARY undefined"
#endif

// thread size checking
#ifndef THREAD0_STACK_SIZE
  #error "THREAD0_STACK_SIZE must be defined"
#elif THREAD0_STACK_SIZE < MIN_STACK_SIZE
  #error "THREAD0_STACK_SIZE is less than minimum size"
#endif

#if (MAX_THREADS >= 2) && !defined(THREAD1_STACK_SIZE)
  #error "THREAD1_STACK_SIZE must be defined"
#elif THREAD1_STACK_SIZE < MIN_STACK_SIZE
  #error "THREAD1_STACK_SIZE is less than minimum size"
#endif

#if (MAX_THREADS >= 3) && !defined(THREAD2_STACK_SIZE)
  #error "THREAD2_STACK_SIZE must be defined"
#elif THREAD2_STACK_SIZE < MIN_STACK_SIZE
  #error "THREAD2_STACK_SIZE is less than minimum size"
#endif

#if (MAX_THREADS >= 4) && !defined(THREAD3_STACK_SIZE)
  #error "THREAD3_STACK_SIZE must be defined"
#elif THREAD3_STACK_SIZE < MIN_STACK_SIZE
  #error "THREAD3_STACK_SIZE is less than minimum size"
#endif

#if (MAX_THREADS >= 5) && !defined(THREAD4_STACK_SIZE)
  #error "THREAD4_STACK_SIZE must be defined"
#elif THREAD4_STACK_SIZE < MIN_STACK_SIZE
  #error "THREAD4_STACK_SIZE is less than minimum size"
#endif

#if (MAX_THREADS >= 6) && !defined(THREAD5_STACK_SIZE)
  #error "THREAD5_STACK_SIZE must be defined"
#elif THREAD5_STACK_SIZE < MIN_STACK_SIZE
  #error "THREAD5_STACK_SIZE is less than minimum size"
#endif

#if (MAX_THREADS >= 7) && !defined(THREAD6_STACK_SIZE)
  #error "THREAD6_STACK_SIZE must be defined"
#elif THREAD6_STACK_SIZE < MIN_STACK_SIZE
  #error "THREAD6_STACK_SIZE is less than minimum size"
#endif

#if (MAX_THREADS == 8) && !defined(THREAD7_STACK_SIZE)
  #error "THREAD7_STACK_SIZE must be defined"
#elif THREAD7_STACK_SIZE < MIN_STACK_SIZE
  #error "THREAD7_STACK_SIZE is less than minimum size"
#endif

/******************************************************************************
 * Stack definitions
 *****************************************************************************/

/** \def TOTAL_STACK_SIZE
 * Sums up the total stack usage for all of the threads in the kernel.
 * \see stack_size
 * \ingroup kernel_implementation
 */
#if MAX_THREADS == 1
  #define TOTAL_STACK_SIZE THREAD0_STACK_SIZE
#elif MAX_THREADS == 2
  #define TOTAL_STACK_SIZE (THREAD0_STACK_SIZE + THREAD1_STACK_SIZE)
#elif MAX_THREADS == 3
  #define TOTAL_STACK_SIZE (THREAD0_STACK_SIZE + THREAD1_STACK_SIZE \
    THREAD2_STACK_SIZE)
#elif MAX_THREADS == 4
  #define TOTAL_STACK_SIZE (THREAD0_STACK_SIZE + THREAD1_STACK_SIZE + \
    THREAD2_STACK_SIZE + THREAD3_STACK_SIZE)
#elif MAX_THREADS == 5
  #define TOTAL_STACK_SIZE (THREAD0_STACK_SIZE + THREAD1_STACK_SIZE + \
    THREAD2_STACK_SIZE + THREAD3_STACK_SIZE + THREAD4_STACK_SIZE)
#elif MAX_THREADS == 6
  #define TOTAL_STACK_SIZE (THREAD0_STACK_SIZE + THREAD1_STACK_SIZE + \
    THREAD2_STACK_SIZE + THREAD3_STACK_SIZE + THREAD4_STACK_SIZE + \
    THREAD5_STACK_SIZE)
#elif MAX_THREADS == 7
  #define TOTAL_STACK_SIZE (THREAD0_STACK_SIZE + THREAD1_STACK_SIZE + \
    THREAD2_STACK_SIZE + THREAD3_STACK_SIZE + THREAD4_STACK_SIZE + \
    THREAD5_STACK_SIZE + THREAD6_STACK_SIZE)
#elif MAX_THREADS == 8
  #define TOTAL_STACK_SIZE (THREAD0_STACK_SIZE + THREAD1_STACK_SIZE + \
    THREAD2_STACK_SIZE + THREAD3_STACK_SIZE + THREAD4_STACK_SIZE + \
    THREAD5_STACK_SIZE + THREAD6_STACK_SIZE + THREAD7_STACK_SIZE)
#else
  // why is this happening??
  #error "Invalid number of threads"
#endif

// check the stack size
#if TOTAL_STACK_SIZE >= TOTAL_RAM_SIZE
  #error "Stacks are too large to fit in RAM"
#endif

/**
 * Sets the starting address of the stack for \ref THREAD0.
 * \ingroup kernel_implementation
 */
#define THREAD0_STACK_BASE ((uint8_t*)RAMEND)
#if MAX_THREADS >= 2
  /**
   * Sets the starting address of the stack for \ref THREAD1.
   * \ingroup kernel_implementation
   */
  #define THREAD1_STACK_BASE \
    ((uint8_t*)(THREAD0_STACK_BASE - THREAD0_STACK_SIZE))
#endif
#if MAX_THREADS >= 3
  /**
   * Sets the starting address of the stack for \ref THREAD2.
   * \ingroup kernel_implementation
   */
  #define THREAD2_STACK_BASE \
    ((uint8_t*)(THREAD1_STACK_BASE - THREAD1_STACK_SIZE))
#endif
#if MAX_THREADS >= 4
  /**
   * Sets the starting address of the stack for \ref THREAD3.
   * \ingroup kernel_implementation
   */
  #define THREAD3_STACK_BASE \
    ((uint8_t*)(THREAD2_STACK_BASE - THREAD2_STACK_SIZE))
#endif
#if MAX_THREADS >= 5
  /**
   * Sets the starting address of the stack for \ref THREAD4.
   * \ingroup kernel_implementation
   */
  #define THREAD4_STACK_BASE \
    ((uint8_t*)(THREAD3_STACK_BASE - THREAD3_STACK_SIZE))
#endif
#if MAX_THREADS >= 6
  /**
   * Sets the starting address of the stack for \ref THREAD5.
   * \ingroup kernel_implementation
   */
  #define THREAD5_STACK_BASE \
    ((uint8_t*)(THREAD4_STACK_BASE - THREAD4_STACK_SIZE))
#endif
#if MAX_THREADS >= 7
  /**
   * Sets the starting address of the stack for \ref THREAD6.
   * \ingroup kernel_implementation
   */
  #define THREAD6_STACK_BASE \
    ((uint8_t*)(THREAD5_STACK_BASE - THREAD5_STACK_SIZE))
#endif
#if MAX_THREADS == 8
  /**
   * Sets the starting address of the stack for \ref THREAD7.
   * \ingroup kernel_implementation
   */
  #define THREAD7_STACK_BASE \
    ((uint8_t*)(THREAD6_STACK_BASE - THREAD6_STACK_SIZE))
#endif

/**
 * Contains pointers to the base of each stack for easier run time access.
 * \ingroup kernel_implementation
 */
static const uint8_t* const kn_stack_base[MAX_THREADS] PROGMEM = {
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
   * Sets pointer to the stack canary for \ref THREAD0.
   * \ingroup kernel_implementation
   */
  #define THREAD0_CANARY_LOC \
    ((uint8_t*)(THREAD0_STACK_BASE - THREAD0_STACK_SIZE + 1))
  #if MAX_THREADS >= 2
    /**
     * Sets pointer to the stack canary for \ref THREAD1.
     * \ingroup kernel_implementation
     */
    #define THREAD1_CANARY_LOC \
      ((uint8_t*)(THREAD1_STACK_BASE - THREAD1_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 3
    /**
     * Sets pointer to the stack canary for \ref THREAD2.
     * \ingroup kernel_implementation
     */
    #define THREAD2_CANARY_LOC \
      ((uint8_t*)(THREAD2_STACK_BASE - THREAD2_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 4
    /**
     * Sets pointer to the stack canary for \ref THREAD3.
     * \ingroup kernel_implementation
     */
    #define THREAD3_CANARY_LOC \
      ((uint8_t*)(THREAD3_STACK_BASE - THREAD3_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 5
    /**
     * Sets pointer to the stack canary for \ref THREAD4.
     * \ingroup kernel_implementation
     */
    #define THREAD4_CANARY_LOC \
      ((uint8_t*)(THREAD4_STACK_BASE - THREAD4_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 6
    /**
     * Sets pointer to the stack canary for \ref THREAD5.
     * \ingroup kernel_implementation
     */
    #define THREAD5_CANARY_LOC \
      ((uint8_t*)(THREAD5_STACK_BASE - THREAD5_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 7
    /**
     * Sets pointer to the stack canary for \ref THREAD6.
     * \ingroup kernel_implementation
     */
    #define THREAD6_CANARY_LOC \
      ((uint8_t*)(THREAD6_STACK_BASE - THREAD6_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS == 8
    /**
     * Sets pointer to the stack canary for \ref THREAD7.
     * \ingroup kernel_implementation
     */
    #define THREAD7_CANARY_LOC \
      ((uint8_t*)(THREAD7_STACK_BASE - THREAD7_STACK_SIZE + 1))
  #endif
  
  /**
   * Contains pointers to each stack's canary location for easier run time 
   * access.
   * \ingroup kernel_implementation
   */
  static const uint8_t* const kn_canary_loc[MAX_THREADS] PROGMEM = {
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

/**
 * Holds the id of the currently executing thread.
 * \ingroup kernel_implementation
 */
thread_id kn_cur_thread;

/**
 * Holds the mask of the currently executing thread.
 * \ingroup kernel_implementation
 */
uint8_t kn_cur_thread_mask;

/**
 * Tracks threads that are inactive.
 * \ingroup kernel_implementation
 */
uint8_t kn_disabled_threads;

/**
 * Tracks threads that have their execution suspended.
 * \ingroup kernel_implementation
 */
uint8_t kn_suspended_threads;

/**
 * Tracks threads that are delayed waiting for a timer.
 * \ingroup kernel_implementation
 */
volatile uint8_t kn_delayed_threads;

/**
 * Holds the saved stack locations for each thread.
 * \ingroup kernel_implementation
 */
uint8_t* kn_stack[MAX_THREADS];

/**
 * Tracks the delay times for each thread.
 * \ingroup kernel_implementation
 */
volatile static uint16_t kn_delay_counter[MAX_THREADS];
  
/**
 * Counts the total system uptime, in milliseconds.
 * \ingroup kernel_implementation
 */
volatile static uint32_t kn_system_counter;

/******************************************************************************
 * Local function declarations
 *****************************************************************************/

/**
 * Sets a thread up for initial entry by popping its parameters off of the 
 * stack and loading them into the registers specified by the ABI of avr-gcc.
 * \see thread_ptr
 * \see kn_create_thread
 * \ingroup kernel_implementation
 */
extern void kn_thread_bootstrap();

/******************************************************************************
 * Function definitions
 *****************************************************************************/

bool kn_create_thread(const thread_id t_id, thread_ptr entry_point, 
  const bool suspended, void* arg)
{
  if ((t_id >= MAX_THREADS) || !entry_point)
  {
    return false;
  }
  
  // set the initial state of the thread's stack
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
  // 2 bytes for the bootstrapper address
  kn_stack[t_id][20] = ((uint16_t)kn_thread_bootstrap) & 0x00FF;
  kn_stack[t_id][19] = ((uint16_t)kn_thread_bootstrap) >> 8;
  // the remaining 18 bytes are popped to restore registers
  // their value doesn't actually matter they just need to be on the stack
  
  if (t_id == kn_cur_thread)
  {
    // call scheduler
  }
  
  return true;
}

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
 * 
 * \ingroup kernel_implementation
 */
__attribute__((naked, section(".init8"), used))
static void kn_init()
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