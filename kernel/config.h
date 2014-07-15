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
 * \brief Contains configuration options for the kernel.
 * \warning This file is used by the assembler, so all defines in this file 
 * must be compile-time constant values.
 * \see kernel_config
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/** 
 * \defgroup kernel_config Kernel Configuration
 * \brief Configuration options for the kernel.
 * 
 * These configuration options allow the kernel to be customized to meet usage 
 * needs.  The number of usable threads may be configured, along with the stack 
 * size for each thread.  Stack canary values may also be enable or disabled 
 * for the kernel.
 * 
 * @{
 */

/**
 * The clock speed of the MCU as used by \c _delay_ms and so on.
 */
#define F_CPU 16000000

/**
 * If defined, \c KERNEL_USE_ASSERT enables assertion checks within the kernel. 
 * When an assertion fails, \ref kn_assertion_failure is called.
 */
#define KERNEL_USE_ASSERT

/**
 * The maximum number of threads that may be enabled simultaneously.  Valid
 * thread id's to use will be from \c THREAD0 to 
 * <tt>THREAD[MAX_THREADS - 1]</tt>.  Value must be in the range [1,8].
 */
#define MAX_THREADS 8

/**
 * If \c KERNEL_USE_STACK_CANARY is defined, the kernel will place a canary 
 * value at the top of each thread's stack, which it uses to determine if the 
 * thread has had a stack overflow.
 */
#define KERNEL_USE_STACK_CANARY

/**
 * The canary value used if \ref KERNEL_USE_STACK_CANARY is defined.  Must be a 
 * 1 byte value.
 */
#define STACK_CANARY 0xAA

/**
 * \defgroup stack_size Thread Stack Sizes
 * 
 * \brief Configuration of stack sizes for each thread.
 * 
 * \c THREAD0_STACK_SIZE must always be defined.  Additional macros must be
 * defined depending on the value of \ref MAX_THREADS, so that there is one 
 * macro for each possible thread.
 * 
 * The sum of the stack sizes for all threads must be less than the size of the 
 * MCU's RAM.  However, realistically, you must account for the sizes of the 
 * .data and .bss segments, as well as the heap size if you intend to use 
 * dynamic allocation.
 * 
 * The absolute minimum stack size is 25 bytes (\ref INITIAL_STACK_USAGE); this 
 * much space is necessary for a thread to be created, and to yield without 
 * overflowing assuming that there is no stack usage within the thread itself.  
 * To do anything useful, however, a minimum stack size of 32 bytes 
 * (\ref MIN_STACK_SIZE) is enforced.
 * 
 * \warning If stack canaries are enabled, they reduce the usable size of each 
 * thread's stack by 1 byte.
 * 
 * @{
 */

/** The size of the stack for \c THREAD0. */
#define THREAD0_STACK_SIZE 64
/** The size of the stack for \c THREAD1. */
#define THREAD1_STACK_SIZE 64
/** The size of the stack for \c THREAD2. */
#define THREAD2_STACK_SIZE 64
/** The size of the stack for \c THREAD3. */
#define THREAD3_STACK_SIZE 64
/** The size of the stack for \c THREAD4. */
#define THREAD4_STACK_SIZE 64
/** The size of the stack for \c THREAD5. */
#define THREAD5_STACK_SIZE 64
/** The size of the stack for \c THREAD6. */
#define THREAD6_STACK_SIZE 64
/** The size of the stack for \c THREAD7. */
#define THREAD7_STACK_SIZE 64

/**
 * @}
 * @}
 */

#endif