avr-kernel
==========

A simple multitasking kernel for written for the ATmega328P (using an Arduino Uno), but should be adaptable to other AVR8 MCU's.  Configurable to allow up to 8 threads, with a custom stack size for each thread and optional canary values to detect stack overflow situations.  See the [Doxygen documentation](http://mbcrawfo.github.io/avr-kernel) for more details.

This kernel is based on a design created by Professor Frank for his CS5549 class, with some added modifications and enhancements by me.
