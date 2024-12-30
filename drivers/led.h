// Description: Interface for driver for the Raspberry Pi Pico's on-board LED.

#ifndef LED_H
#define LED_H

void configure_led( void );
void turn_on_led( void );
void turn_off_led( void );
void toggle_led( void );

#endif
