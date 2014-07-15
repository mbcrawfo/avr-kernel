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
 * \brief Contains debugging tools.
 */

#ifndef KERNEL_DEBUG_H_
#define KERNEL_DEBUG_H_

#include "kernel_types.h"
#include "config.h"

/**
 * \addtogroup kernel_interface
 * @{
 */

#ifdef KERNEL_USE_STACK_CANARY
/**
 * A user supplied function that is called when a stack overflow is detected. 
 * Used only if \ref KERNEL_USE_STACK_CANARY is defined.
 * 
 * \param[in] t_id The id of the active thread when the stack overflow was 
 * detected. This does not necessarily mean that corruption is limited to this 
 * thread's stack.
 * 
 * \warning If you return from this function, the kernel scheduler will attempt 
 * to continue, but you do so at your own risk.
 */
extern void kn_stack_overflow(const thread_id t_id);
#endif

#ifdef KERNEL_USE_ASSERT
/**
 * If the given expression evaluates to false, calls \ref kn_assertion_failure.
 */
#define kn_assert(expr) \
  do { \
    if (!(expr)) \
      kn_assertion_failure(#expr, __FILE__, __BASE_FILE__, __LINE__); \
  } while (0)
    
/**
 * A user supplied function that is called when an assertion fails. Used only 
 * if \ref KERNEL_USE_ASSERT is defined.
 * 
 * \param[in] expr The assertion expression that failed.
 * \param[in] file The name of the file where the assertion failed.
 * \param[in] base_file The file being compiled when the assertion failed.
 * \param[in] line The line number of \c file where the assertion failed.
 * 
 * \warning This function should call \c exit or otherwise be prevented from 
 * returning.
 */
extern void kn_assertion_failure(const char* expr, const char* file,
                                 const char* base_file, const int line);

#else
  #define kn_assert(expr) ((void)0)
#endif

/**
 * @}
 */

#endif