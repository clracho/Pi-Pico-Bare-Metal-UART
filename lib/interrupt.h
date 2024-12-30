// Description: Uses inline assembly to enable all maskable interrupts via PRIMASK register. Also
// provides preprocessor directives for enabling and disabling interrupts.

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

#define enable_irq() asm ("CPSIE I")
#define disable_irq() asm ("CPSID I")
#define set_primask(V) asm ("MSR primask, %0" : : "r" (V) )

static uint32_t get_primask()
{
	uint32_t retval = 0;
	asm volatile ("MSR primask, %0" : : "r" (retval) );
	return retval;
}

#endif