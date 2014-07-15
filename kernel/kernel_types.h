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
 * \brief Contains type definitions used by the kernel.
 */

#ifndef KERNEL_TYPES_H_
#define KERNEL_TYPES_H_

#include <stdbool.h>
#include <stdint.h>

/** \cond */
#ifndef NULL
#define NULL ((void*)0)
#endif
/** \endcond */

/**
 * \addtogroup kernel_interface
 * @{
 */

/** Defines an explicit type to use for thread id's. */
typedef enum
{
  THREAD0 = 0,
  THREAD1,
  THREAD2,
  THREAD3,
  THREAD4,
  THREAD5,
  THREAD6,
  THREAD7
} thread_id;

/**
 * The function type for threads used by the kernel.  To reduce code size and 
 * unnecessary stack usage, thread functions should be given the gcc attribute 
 * \c OS_task. While thread functions do not need to worry about saving or 
 * restoring any registers, it is not recommended to give them the \c naked 
 * attribute, because the compiler may generate code that assumes the stack has 
 * been set up by a function prologue.
 * 
 * \param[in] my_id The thread id of this thread.
 * \param[in] arg A parameter to pass information to the thread.
 * 
 * \warning Do not allow any thread to return: doing so will break things...
 */
typedef void (*thread_ptr)(const thread_id my_id, void* arg);

/**
 * @}
 */

#endif