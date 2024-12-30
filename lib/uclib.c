// Description: Implementation of a hex dump, print string, and print hex function for a Raspberry Pi Pico.

#include <uclib.h>
#include <stdint.h>
#include <usbcdc.h>

void hex_dump( uint32_t start, uint8_t bytes )
{
	uint32_t aligned_start = start & ~0x7;
	uint8_t* ptr = (uint8_t *) aligned_start;
	for (uint8_t byte_index = 0; byte_index < bytes; byte_index += 8)
	{
		print_string("\n\r0x");
		print_hex(8, aligned_start + byte_index);
		print_string("  ");
		
		for (uint8_t i = 0; i < 8; i++)
		{
			print_hex(2, *(ptr + byte_index + i));
			usbcdc_putchar(' ');
		}
	}
}

void print_string( char const * str )
{
	while ( *str && usbcdc_putchar(*str++))
		continue;
}

void print_hex( uint8_t num_digits, uint32_t value )
{
	int8_t i;
	if (num_digits > 8)
		num_digits = 8;
	
	for ( i = num_digits - 1; i >= 0; i-- )
	{
		uint8_t hex_digit=(value>>4*i)&0xf;
		usbcdc_putchar(hex_digit+((hex_digit < 10)?'0':'A'-10));
	}
}

uint16_t div10(uint16_t x)
{
	return (x*0x199a)>>16;	
}

uint16_t mod10(uint16_t x)
{
	return x-div10(x)*10;
}