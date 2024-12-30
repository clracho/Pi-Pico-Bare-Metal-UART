// Description: File for interfacing with the systick driver.

#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdbool.h>

void configure_systick( void );
_Bool systick_has_fired( void );
_Bool systick_register_callback(void (*p_fn)());

#endif
