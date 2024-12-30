// Description: Driver creates a watchdog with a 1us tick period. Can be configured with a reload
// interval using configure_watchdog() and fed using feed_the_watchdog().

#include "watchdog.h"
#include <rp2040/watchdog.h>
#include <rp2040/psm.h>
#include "stdint.h"

static uint32_t reload_value;

void configure_watchdog( uint32_t reload )
{
	watchdog->ctrl &= ~(WATCHDOG_CTRL_ENABLE_MASK);
	psm->wdsel = PSM_WDSEL_CLOCKS_MASK | PSM_WDSEL_RESETS_MASK |
				PSM_WDSEL_BUSFABRIC_MASK | PSM_WDSEL_ROM_MASK | 
				PSM_WDSEL_SRAM0_MASK | PSM_WDSEL_SRAM1_MASK | 
				PSM_WDSEL_SRAM2_MASK | PSM_WDSEL_SRAM3_MASK | 
				PSM_WDSEL_SRAM4_MASK | PSM_WDSEL_SRAM5_MASK | 
				PSM_WDSEL_XIP_MASK | PSM_WDSEL_VREG_AND_CHIP_RESET_MASK | 
				PSM_WDSEL_SIO_MASK | PSM_WDSEL_PROC0_MASK | 
				PSM_WDSEL_PROC1_MASK;
	watchdog->tick = 0xC | WATCHDOG_TICK_ENABLE_MASK;
	reload_value = reload * 2;
	feed_the_watchdog();
	watchdog->ctrl |= WATCHDOG_CTRL_ENABLE_MASK;
}

void feed_the_watchdog( void )
{
	watchdog->load = reload_value; 	
}
