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
 * \brief Defines the stack locations for the kernel.
 * \see kernel_implementation
 */

#ifndef STACKS_H_
#define STACKS_H_

#include "config.h"
#include <avr/io.h>

/**
 * The minimum size of each stack.
 * \see stack_size
 * \ingroup kernel_implementation
 */
#define MIN_STACK_SIZE 32

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
 * \ingroup kernel_implementation
 */
#define INITIAL_STACK_USAGE 25

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
 * Sums up the total stack usage for all of the user threads.
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

/** \def STACK_CAST
 * Conditionally defined macro to allow stack locations to be used in 
 * assembly while avoiding compiler warnings.
 * \ingroup kernel_implementation
 */
#ifdef __ASSEMBLER__
  #define STACK_CAST(x) (x)
#else
  #define STACK_CAST(x) ((uint8_t*)x)
#endif

/**
 * Sets the starting address of the stack for \ref THREAD0.
 * \ingroup kernel_implementation
 */
#define THREAD0_STACK_BASE STACK_CAST(RAMEND)
#if MAX_THREADS >= 2
  /**
   * Sets the starting address of the stack for \ref THREAD1.
   * \ingroup kernel_implementation
   */
  #define THREAD1_STACK_BASE \
    STACK_CAST(THREAD0_STACK_BASE - THREAD0_STACK_SIZE)
#endif
#if MAX_THREADS >= 3
  /**
   * Sets the starting address of the stack for \ref THREAD2.
   * \ingroup kernel_implementation
   */
  #define THREAD2_STACK_BASE \
    STACK_CAST(THREAD1_STACK_BASE - THREAD1_STACK_SIZE)
#endif
#if MAX_THREADS >= 4
  /**
   * Sets the starting address of the stack for \ref THREAD3.
   * \ingroup kernel_implementation
   */
  #define THREAD3_STACK_BASE \
    STACK_CAST(THREAD2_STACK_BASE - THREAD2_STACK_SIZE)
#endif
#if MAX_THREADS >= 5
  /**
   * Sets the starting address of the stack for \ref THREAD4.
   * \ingroup kernel_implementation
   */
  #define THREAD4_STACK_BASE \
    STACK_CAST(THREAD3_STACK_BASE - THREAD3_STACK_SIZE)
#endif
#if MAX_THREADS >= 6
  /**
   * Sets the starting address of the stack for \ref THREAD5.
   * \ingroup kernel_implementation
   */
  #define THREAD5_STACK_BASE \
    STACK_CAST(THREAD4_STACK_BASE - THREAD4_STACK_SIZE)
#endif
#if MAX_THREADS >= 7
  /**
   * Sets the starting address of the stack for \ref THREAD6.
   * \ingroup kernel_implementation
   */
  #define THREAD6_STACK_BASE \
    STACK_CAST(THREAD5_STACK_BASE - THREAD5_STACK_SIZE)
#endif
#if MAX_THREADS == 8
  /**
   * Sets the starting address of the stack for \ref THREAD7.
   * \ingroup kernel_implementation
   */
  #define THREAD7_STACK_BASE \
    STACK_CAST(THREAD6_STACK_BASE - THREAD6_STACK_SIZE)
#endif

#ifdef USE_STACK_CANARY
  /**
   * Sets pointer to the stack canary for \ref THREAD0.
   * \ingroup kernel_implementation
   */
  #define THREAD0_CANARY_LOC \
    STACK_CAST(THREAD0_STACK_BASE - THREAD0_STACK_SIZE + 1)
  #if MAX_THREADS >= 2
    /**
     * Sets pointer to the stack canary for \ref THREAD1.
     * \ingroup kernel_implementation
     */
    #define THREAD1_CANARY_LOC \
      STACK_CAST(THREAD1_STACK_BASE - THREAD1_STACK_SIZE + 1)
  #endif
  #if MAX_THREADS >= 3
    /**
     * Sets pointer to the stack canary for \ref THREAD2.
     * \ingroup kernel_implementation
     */
    #define THREAD2_CANARY_LOC \
      STACK_CAST(THREAD2_STACK_BASE - THREAD2_STACK_SIZE + 1)
  #endif
  #if MAX_THREADS >= 4
    /**
     * Sets pointer to the stack canary for \ref THREAD3.
     * \ingroup kernel_implementation
     */
    #define THREAD3_CANARY_LOC \
      STACK_CAST(THREAD3_STACK_BASE - THREAD3_STACK_SIZE + 1)
  #endif
  #if MAX_THREADS >= 5
    /**
     * Sets pointer to the stack canary for \ref THREAD4.
     * \ingroup kernel_implementation
     */
    #define THREAD4_CANARY_LOC \
      STACK_CAST(THREAD4_STACK_BASE - THREAD4_STACK_SIZE + 1)
  #endif
  #if MAX_THREADS >= 6
    /**
     * Sets pointer to the stack canary for \ref THREAD5.
     * \ingroup kernel_implementation
     */
    #define THREAD5_CANARY_LOC \
      STACK_CAST(THREAD5_STACK_BASE - THREAD5_STACK_SIZE + 1)
  #endif
  #if MAX_THREADS >= 7
    /**
     * Sets pointer to the stack canary for \ref THREAD6.
     * \ingroup kernel_implementation
     */
    #define THREAD6_CANARY_LOC \
      STACK_CAST(THREAD6_STACK_BASE - THREAD6_STACK_SIZE + 1)
  #endif
  #if MAX_THREADS == 8
    /**
     * Sets pointer to the stack canary for \ref THREAD7.
     * \ingroup kernel_implementation
     */
    #define THREAD7_CANARY_LOC \
      STACK_CAST(THREAD7_STACK_BASE - THREAD7_STACK_SIZE + 1)
  #endif
#endif

#endif