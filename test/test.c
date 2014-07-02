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

#include "kernel.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void threadA(const thread_id my_id, void* arg) 
  __attribute__((OS_task));
void threadB(const thread_id my_id, void* arg) 
  __attribute__((OS_task));

#pragma GCC diagnostic ignored "-Wmain"
void main() __attribute__((OS_main));
void main()
{
  DDRB |= _BV(5);  
  sei();
  
  kn_replace_self(&threadA, false, NULL);
}

void threadA(const thread_id my_id, void* arg)
{
  while(1)
  {
    PORTB ^= _BV(5);
    kn_sleep_long(70, 0);
    PORTB ^= _BV(5);
    kn_sleep_long(5, 0);
  }
}

void threadB(const thread_id my_id, void* arg)
{
  while (1)
  {
    kn_yield();
  }
}