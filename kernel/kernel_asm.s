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
 * \brief Contains the parts of the kernel implemented in assembly.
 * \see kernel_implementation
 */

/** \cond */
#define __SFR_OFFSET 0
/** \endcond */

#include "config.h"
#include <avr/io.h>

// external symbols from kernel.c
.extern kn_canary_loc
.extern kn_cur_thread
.extern kn_cur_thread_mask
.extern kn_disabled_threads
.extern kn_suspended_threads
.extern kn_delayed_threads
.extern kn_stack

  .section .text

// void kn_thread_bootstrap()
.global kn_thread_bootstrap
kn_thread_bootstrap:
  pop r24
  pop r22
  pop r23  
  ret