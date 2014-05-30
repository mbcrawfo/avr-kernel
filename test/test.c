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

void threadA(const thread_id my_id, void* arg) __attribute__((OS_task, noreturn));
void threadB(const thread_id my_id, void* arg) __attribute__((OS_task, noreturn));

int main() __attribute__((OS_main));
int main()
{
  kn_create_thread(THREAD1, &threadA, false, (void*)0x1234);
  kn_create_thread(THREAD2, &threadB, false, NULL);
  
  while (1) {}  
}

void threadA(const thread_id my_id, void* arg)
{
  while (1) {}
}

void threadB(const thread_id my_id, void* arg)
{
  while (1) {}
}