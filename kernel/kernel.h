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
 * \brief Documents the main interface of the kernel.
 * \see kernel_interface
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdbool.h>
#include <stdint.h>

#ifndef NULL
  /** \cond */
  #define NULL ((void*)0)
  /** \endcond */
#endif

/**
 * \defgroup kernel_interface Kernel Interface
 * \brief Contains the public interface of the kernel.
 * 
 * @{
 */

/** Defines an explicit type to use for thread id's. */
typedef uint8_t thread_id;

/** The id for the first thread. */
#define THREAD0 0
/** The id for the second thread. */
#define THREAD1 1
/** The id for the third thread. */
#define THREAD2 2
/** The id for the fourth thread. */
#define THREAD3 3
/** The id for the fifth thread. */
#define THREAD4 4
/** The id for the sixth thread. */
#define THREAD5 5
/** The id for the seventh thread. */
#define THREAD6 6
/** The id for the eighth thread. */
#define THREAD7 7

/**
 * The function type for threads used by the kernel.  To reduce code size and 
 * unnecessary stack usage, thread functions should be given the gcc attribute 
 * \c OS_task. While thread functions do not need to worry about saving or 
 * restoring any registers, it is not recommended to give them the \c naked 
 * attribute, because the compiler may generate code that assumes a prologue 
 * has set up the stack.
 * 
 * \param[in] my_id The thread id of this thread.
 * \param[in] arg A parameter to pass information to the thread.
 * 
 * \warning Do not allow any thread to return: doing so will break your program.
 */
typedef void (*thread_ptr)(const thread_id my_id, void* arg);

/**
 * Creates a new thread of operation within the kernel.
 * 
 * \param[in] t_id The id of the new thread. Must be a valid thread identifier. 
 * If the thread id is an enabled thread, that thread will be replaced.
 * \param[in] entry_point The function that will be run as the new thread. Must 
 * not be null.
 * \param[in] suspended The initial state of the new thread. If true, the 
 * thread will not run until it is manually resumed.
 * \param[in] arg The parameter that will be passed to the function.
 * 
 * \return True if the thread was created successfully.
 * 
 * \warning If \c t_id is the currently active thread, this function does not 
 * return.
 */
extern bool kn_create_thread(const thread_id t_id, thread_ptr entry_point, 
                             const bool suspended, void* arg);
/**
 * Replaces the calling thread with a new thread. Basically is just a wrapper 
 * for the \ref kn_create_thread that automatically supplies the id of the 
 * calling thread. See \ref kn_create_thread for behavior and parameter info.
 * 
 * \warning This thread does not return.
 * \see kn_create_thread
 */
static inline bool kn_replace_self(thread_ptr entry_point, const bool suspended,
                                   void* arg);
  
/**
 * Allows a thread to yield execution to the scheduler.  Will return when the 
 * scheduler selects the calling thread for execution again.
 */
extern void kn_yield();

/**
 * Returns the id of the currently running thread.
 */
static inline thread_id kn_current_thread();

/**
 * Returns true if the specified thread is currently enabled. This does not 
 * necessarily mean that the thread is currently active, as it may be 
 * suspended or sleeping.
 */
extern bool kn_thread_enabled(const thread_id t_id);

/**
 * Returns true if the specified thread is enabled, but suspended.
 */
extern bool kn_thread_suspended(const thread_id t_id);

/**
 * Returns true if the specified thread is enabled, but currently sleeping.
 */
extern bool kn_thread_sleeping(const thread_id t_id);

/**
 * Disables the specified thread. If \c t_id is invalid, does nothing. After a 
 * thread has been disable, you must call \ref kn_create_thread to restart it 
 * or replace it with a new thread. If you wish to "pause" a thread with the 
 * ability to resume it at a later time, use \ref kn_suspend.
 * 
 * \warning If kn_disable is the calling thread, this function does not return.
 */
extern void kn_disable(const thread_id t_id);

/**
 * Disables the calling thread.
 * 
 * \warning Does not return.
 * \see kn_disable
 */
static inline void kn_disable_self();

/**
 * Resumes the specified thread, so that the scheduler may select it for 
 * execution. If \c t_id is invalid, does nothing.
 */
extern void kn_resume(const thread_id t_id);

/**
 * Suspends the specified thread. If \c t_id is invalid, does nothing. If 
 * \c t_id is the calling thread, this function yields and will return after 
 * the thread has been resumed and selected for execution again.
 */
extern void kn_suspend(const thread_id t_id);

/**
 * Suspends the calling thread.
 * 
 * \see kn_suspend
 */
static inline void kn_suspend_self();

/**
 * Converts a bit number to a bit mask.  For example, bit 0 produces the mask 
 * 0x01.  Implemented using a lookup table for better performance than a loop 
 * with bitwise operations. Technically this function isn't part of the kernel, 
 * it's just made available since it provides a commonly used functionality.
 * 
 * \param[in] bit_num The bit number to be converted to a mask. Zero-indexed. 
 * Valid values are in the range [0,7].
 * 
 * \return The bit mask corresponding to \c bit_num.
 */
static inline uint8_t bit_to_mask(uint8_t bit_num) __attribute__((pure));

/**
 * @}
 */

// inline functions definitions
#include "core/kernel-inl.h"

#endif