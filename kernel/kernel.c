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
 * \ingroup kernel_implementation
 * The minimum usable size of each stack.
 * \see stack_size
 */
#define MIN_STACK_SIZE 20

/**
 * \ingroup kernel_implementation
 * The total size of the RAM available on the MCU.
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
 * \ingroup kernel_implementation
 * Sums up the total stack usage for all of the threads in the kernel.
 * \see stack_size
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
 * \ingroup kernel_implementation
 * Sets the starting address of the stack for \ref THREAD0.
 */
#define THREAD0_STACK_BASE ((uint8_t*)RAMEND)
#if MAX_THREADS >= 2
  /**
   * \ingroup kernel_implementation
   * Sets the starting address of the stack for \ref THREAD1.
   */
  #define THREAD1_STACK_BASE \
    ((uint8_t*)(THREAD0_STACK_BASE - THREAD0_STACK_SIZE))
#endif
#if MAX_THREADS >= 3
  /**
   * \ingroup kernel_implementation
   * Sets the starting address of the stack for \ref THREAD2.
   */
  #define THREAD2_STACK_BASE \
    ((uint8_t*)(THREAD1_STACK_BASE - THREAD1_STACK_SIZE))
#endif
#if MAX_THREADS >= 4
  /**
   * \ingroup kernel_implementation
   * Sets the starting address of the stack for \ref THREAD3.
   */
  #define THREAD3_STACK_BASE \
    ((uint8_t*)(THREAD2_STACK_BASE - THREAD2_STACK_SIZE))
#endif
#if MAX_THREADS >= 5
  /**
   * \ingroup kernel_implementation
   * Sets the starting address of the stack for \ref THREAD4.
   */
  #define THREAD4_STACK_BASE \
    ((uint8_t*)(THREAD3_STACK_BASE - THREAD3_STACK_SIZE))
#endif
#if MAX_THREADS >= 6
  /**
   * \ingroup kernel_implementation
   * Sets the starting address of the stack for \ref THREAD5.
   */
  #define THREAD5_STACK_BASE \
    ((uint8_t*)(THREAD4_STACK_BASE - THREAD4_STACK_SIZE))
#endif
#if MAX_THREADS >= 7
  /**
   * \ingroup kernel_implementation
   * Sets the starting address of the stack for \ref THREAD6.
   */
  #define THREAD6_STACK_BASE \
    ((uint8_t*)(THREAD5_STACK_BASE - THREAD5_STACK_SIZE))
#endif
#if MAX_THREADS == 8
  /**
   * \ingroup kernel_implementation
   * Sets the starting address of the stack for \ref THREAD7.
   */
  #define THREAD7_STACK_BASE \
    ((uint8_t*)(THREAD6_STACK_BASE - THREAD6_STACK_SIZE))
#endif

/**
 * \ingroup kernel_implementation
 * Contains pointers to the base of each stack for easier run time access.
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
   * \ingroup kernel_implementation
   * Sets pointer to the stack canary for \ref THREAD0.
   */
  #define THREAD0_CANARY_LOC \
    ((uint8_t*)(THREAD0_STACK_BASE - THREAD0_STACK_SIZE + 1))
  #if MAX_THREADS >= 2
    /**
     * \ingroup kernel_implementation
     * Sets pointer to the stack canary for \ref THREAD1.
     */
    #define THREAD1_CANARY_LOC \
      ((uint8_t*)(THREAD1_STACK_BASE - THREAD1_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 3
    /**
     * \ingroup kernel_implementation
     * Sets pointer to the stack canary for \ref THREAD2.
     */
    #define THREAD2_CANARY_LOC \
      ((uint8_t*)(THREAD2_STACK_BASE - THREAD2_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 4
    /**
     * \ingroup kernel_implementation
     * Sets pointer to the stack canary for \ref THREAD3.
     */
    #define THREAD3_CANARY_LOC \
      ((uint8_t*)(THREAD3_STACK_BASE - THREAD3_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 5
    /**
     * \ingroup kernel_implementation
     * Sets pointer to the stack canary for \ref THREAD4.
     */
    #define THREAD4_CANARY_LOC \
      ((uint8_t*)(THREAD4_STACK_BASE - THREAD4_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 6
    /**
     * \ingroup kernel_implementation
     * Sets pointer to the stack canary for \ref THREAD5.
     */
    #define THREAD5_CANARY_LOC \
      ((uint8_t*)(THREAD5_STACK_BASE - THREAD5_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS >= 7
    /**
     * \ingroup kernel_implementation
     * Sets pointer to the stack canary for \ref THREAD6.
     */
    #define THREAD6_CANARY_LOC \
      ((uint8_t*)(THREAD6_STACK_BASE - THREAD6_STACK_SIZE + 1))
  #endif
  #if MAX_THREADS == 8
    /**
     * \ingroup kernel_implementation
     * Sets pointer to the stack canary for \ref THREAD7.
     */
    #define THREAD7_CANARY_LOC \
      ((uint8_t*)(THREAD7_STACK_BASE - THREAD7_STACK_SIZE + 1))
  #endif
  
  /**
   * \ingroup kernel_implementation
   * Contains pointers to each stack's canary location for easier run time 
   * access.
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
 * \ingroup kernel_implementation
 * Holds the id of the currently executing thread.
 */
thread_id kn_cur_thread;

/**
 * \ingroup kernel_implementation
 * Holds the mask of the currently executing thread.
 */
uint8_t kn_cur_thread_mask;

/**
 * \ingroup kernel_implementation
 * Tracks threads that are inactive.
 */
uint8_t kn_disabled_threads;

/**
 * \ingroup kernel_implementation
 * Tracks threads that have their execution suspended.
 */
uint8_t kn_suspended_threads;

/**
 * \ingroup kernel_implementation
 * Tracks threads that are delayed waiting for a timer.
 */
volatile uint8_t kn_delayed_threads;

/**
 * \ingroup kernel_implementation
 * Holds the saved stack locations for each thread.
 */
uint8_t* kn_stack[MAX_THREADS];

/**
 * \ingroup kernel_implementation
 * Tracks the delay times for each thread.
 */
volatile static uint16_t kn_delay_counter[MAX_THREADS];
  
/**
 * \ingroup kernel_implementation
 * Counts the total system uptime, in milliseconds.
 */
volatile static uint32_t kn_system_counter;

/******************************************************************************
 * Local function declarations
 *****************************************************************************/

/******************************************************************************
 * Function definitions
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
 * \ingroup kernel_implementation
 * 
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
__attribute__((naked))
__attribute__((section(".init8")))
__attribute__((used))
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