// Description: Interrupt based SYSTICK driver that provides a timer. Has a function for
// configuring the SYSTICK and determining if the systick has fired.

#include "systick.h"
#include "interrupt.h"
#include <rp2040/m0plus.h>
#include <stdbool.h>
#include <stdint.h>

#define SYSTICK_FREQ_HZ 1000
#define SYSTICK_TOP (SYS_CLOCK/SYSTICK_FREQ_HZ)

static volatile _Bool systick_fired_flag = false;

void __attribute__((isr)) SystickHandler();

#ifndef NUM_CALLBACKS
#define NUM_CALLBACKS 5
#endif

static void (*callback[NUM_CALLBACKS])();
static uint32_t num_callbacks;
void configure_systick( void )
{
	m0plus->syst_rvr = SYSTICK_TOP;
	m0plus->syst_cvr = 0;
	((void (**)()) m0plus->vtor)[15] = SystickHandler;
	m0plus->shpr3 = (m0plus->shpr3 & ~M0PLUS_SHPR3_PRI_15_MASK)
		| M0PLUS_SHPR3_PRI_15(0);
	m0plus->syst_csr = M0PLUS_SYST_CSR_CLKSOURCE(1)
		| M0PLUS_SYST_CSR_ENABLE_MASK
		| M0PLUS_SYST_CSR_TICKINT_MASK;
}

_Bool systick_has_fired( void )
{
	uint32_t primask = get_primask();
	disable_irq();
	_Bool retval = systick_fired_flag;
	systick_fired_flag = false;
	set_primask(primask);
	return retval;
}

_Bool systick_register_callback(void (*p_fn)())
{
	if (NUM_CALLBACKS == num_callbacks)
		return false;
	callback[num_callbacks++] = p_fn;
	return true;
}

void __attribute__((isr)) SystickHandler()
{
	systick_fired_flag = true;
	for(uint32_t i = 0; i < num_callbacks; i++)
		callback[i]();
}
