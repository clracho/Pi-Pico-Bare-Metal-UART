// Description: File for interfacing with the watchdog driver.

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>

void configure_watchdog( uint32_t reload );
void feed_the_watchdog( void );

#endif
