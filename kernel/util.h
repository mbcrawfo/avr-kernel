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
 * \brief Contains misc. utility functions.
 */

#ifndef UTIL_H_
#define UTIL_H_

#include "kernel_types.h"
#include "kernel_debug.h"

/**
 * Converts a bit number to a bit mask.  For example, bit 0 produces the mask 
 * 0x01.  Implemented using a lookup table for better performance than a loop 
 * with bitwise operations.
 * 
 * \param[in] bit_num The bit number to be converted to a mask. Zero-indexed. 
 * Valid values are in the range [0,7].
 * 
 * \return The bit mask corresponding to \c bit_num.
 */
static inline uint8_t bit_to_mask(uint8_t bit_num) __attribute__((pure));
uint8_t bit_to_mask(uint8_t bit_num)
{
  extern const uint8_t kn_bitmasks[8] PROGMEM;
  kn_assert(bit_num < 8);
  return pgm_read_byte(&kn_bitmasks[bit_num]);
}

#endif