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
 * Converts a bit number to a bit mask.  For example, bit 0 produces the mask 
 * 0x01.  Implemented using a lookup table for better performance than a loop 
 * with bitwise operations. Technically this function isn't part of the kernel, 
 * it's just made available since it provides a commonly used functionality.
 * 
 * \param[in] bit_num The bit number to be converted to a mask. Zero-indexed. 
 * Valid values are in the range [0,7].
 * 
 * \return The bit mask corresponding to \c bit_num. If \c bit_num is invalid, 
 * 0 is returned.
 */
extern uint8_t bit_to_mask(uint8_t bit_num);

/**
 * @}
 */

#endif