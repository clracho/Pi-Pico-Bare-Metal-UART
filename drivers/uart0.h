// Description: Interface for driver for the UART peripheral on the rp2040 processor.

#ifndef UART0_H
#define UART0_H

#include <stdbool.h>

void configure_uart(void);
void uart0_putchar(char data);
_Bool uart0_getchar(char *data);

#endif