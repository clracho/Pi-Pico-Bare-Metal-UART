// Description: File to interface with hex dump, print string, and print hex functions defined in uclib.c.

#ifndef UCLIB_H
#define UCLIB_H

#include <stdint.h>

void hex_dump( uint32_t start, uint8_t bytes );
void print_string( char const * str );
void print_hex( uint8_t num_digits, uint32_t value );
uint16_t div10( uint16_t x );
uint16_t mod10( uint16_t x );

#endif
