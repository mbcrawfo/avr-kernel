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

/** \mainpage Overview
 * avr-kernel is a lightweight kernel for the AtMega328p microcontroller, 
 * capable of supporting up to 8 threads.  When compiled with full support 
 * for 8 threads, the kernel uses 41 bytes of RAM and around 1 KB of program 
 * memory.
 * 
 * See \ref kernel_config for the user-configurable options available, and 
 * \ref kernel_interface for the main interface.
 * 
 * The kernel uses a fairly basic round-robin cooperative scheduler.  Each 
 * thread "owns" the processor and must yield to the kernel so that other 
 * threads may execute. After all of the other ready threads have been given a 
 * chance to execute, the original thread is selected again.  In other words, 
 * when a thread yields to the kernel, there are no guarantees of how quickly 
 * the thread will be executed again.
 * 
 * Threads exist in one of four possible states:
 * -# \b Disabled  The thread is totally inactive and exists in an invalid 
 *    state.  It will not execute until a new thread is created in its place.  
 *    Initially, the \c main function is entered as \c THREAD0, and all other 
 *    threads are disabled.
 * -# \b Suspended The thread exists in a valid state, but it does not execute. 
 *    The thread must be resumed from another thread or an interrupt to allow 
 *    execution to continue.
 * -# \b Sleeping  Sleeping threads exist in a valid state, but their execution 
 *    is suspended and they will automatically be resumed by the kernel when 
 *    their sleep time is elapsed.
 * -# \b Active  A thread that is not in any of the above states is active, and 
 *    will be executed by the kernel scheduler.
 * 
 * Each thread runs on its own stack, and the stack size for each thread may 
 * be configured in the kernel options.  However, kernel functions and 
 * interrupts are both executed on the stack of the thread that is active when 
 * they are called.  If you wish to use \c malloc when using this kernel, you 
 * will need to include \c core/stacks.h, and set <tt>__malloc_heap_end = 
 * RAMEND - TOTAL_STACK_SIZE</tt> early in your program initialization 
 * (see http://www.nongnu.org/avr-libc/user-manual/malloc.html).
 * 
 * Kernel initialization occurs automatically before \c main is called.  The 
 * only user action necessary is to enable interrupts, as the kernel uses 
 * \c Timer0 to provide a millisecond counter and implement the sleep functions.
 * 
 * To assist with debugging, the user may enable canary values to detect when 
 * a thread has overflowed its stack.  Additionally an assertion macro may 
 * be enabled to provide error checking for the parameters of the kernel 
 * functions. 
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include "kernel_types.h"

/**
 * \defgroup kernel_interface Kernel Interface
 * \brief Contains the public interface of the kernel.
 * 
 * @{
 */

/**
 * Creates a new thread of operation within the kernel.
 * 
 * \param[in] t_id The id of the new thread. If the thread id is an enabled 
 * thread, that thread will be replaced.
 * \param[in] entry_point The function that will be run as the new thread.
 * \param[in] suspended The initial state of the new thread. If true, the 
 * thread will not run until it is manually resumed.
 * \param[in] arg The parameter that will be passed to the function.
 * 
 * \warning If \c t_id is the currently active thread, this function does not 
 * return.
 */
extern void kn_create_thread(const thread_id t_id, thread_ptr entry_point, 
                             const bool suspended, void* arg);
/**
 * Replaces the calling thread with a new thread. Basically is just a wrapper 
 * for the \ref kn_create_thread that automatically supplies the id of the 
 * calling thread. See \ref kn_create_thread for behavior and parameter info.
 * 
 * \warning Does not return.
 * \see kn_create_thread
 */
static inline void kn_replace_self(thread_ptr entry_point, const bool suspended,
                                   void* arg);
  
/**
 * Allows a thread to yield execution to the scheduler.  Will return when the 
 * scheduler selects the calling thread for execution again.
 */
extern void kn_yield();

/**
 * Allows a thread to sleep for a certain amount of time. For sleep times 
 * longer than 65 seconds, use \ref kn_sleep_long.
 * 
 * \param[in] millis The amount of time in milliseconds to sleep.
 */
extern void kn_sleep(const uint16_t millis);

/**
 * Allows a thread to sleep for longer time periods than \ref kn_sleep. The 
 * maximum sleep time is approximately 49 days.
 * 
 * \param[in] millis The number of milliseconds to sleep.
 */
extern void kn_sleep_long(uint32_t millis);

/**
 * Returns the system timer, in milliseconds. This value will overflow after 
 * 49 days.
 */
extern uint32_t kn_millis();

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
 * Disables the specified thread. After a thread has been disabled, you must 
 * call \ref kn_create_thread to restart it or replace it with a new thread. If 
 * you wish to "pause" a thread with the ability to resume it at a later time, 
 * use \ref kn_suspend.
 * 
 * \warning If t_id is the calling thread, this function does not return.
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
 * execution.
 */
extern void kn_resume(const thread_id t_id);

/**
 * Suspends the specified thread. If t_id is the calling thread, this function 
 * yields and will return after the thread has been resumed and selected for 
 * execution again.
 */
extern void kn_suspend(const thread_id t_id);

/**
 * Suspends the calling thread.
 * 
 * \see kn_suspend
 */
static inline void kn_suspend_self();

/**
 * @}
 */

// inline function definitions
#include "core/kernel-inl.h"

#endif