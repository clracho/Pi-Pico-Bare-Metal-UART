// Description: Main program that controls the duty-cycle and frequency of a PWM peripheral via a UART 
//				interface. When two Raspberry Pi Picos are programmed with this program, they can send 
//				commands to control eachother's LEDs. Keyboard presses from the user initiate transmit
// 				commands. Keys "1", "2", and "3" correspond to the duty-cycle of the LED, while keys
//				"S" and "F" correspond to the frequency of the flashing LED.
//				Additionally, a push-button enables/disables the use of the on-board LED.

#include "systick.h"
#include "watchdog.h"
#include "interrupt.h"
#include "usbcdc.h"
#include "uclib.h"
#include "pbinterrupt.h"
#include "led.h"
#include "pwm0.h"
#include "uart0.h"
#include <stdbool.h>
#include <stdint.h>

void onboard_led_control();
void led_pwm_control_fsm();

int main ( void )
{	
	disable_irq();
	configure_systick();
	configure_watchdog(1500);
	configure_usbcdc();
	systick_register_callback(led_pwm_control_fsm);
	systick_register_callback(pushbutton_debounce_fsm);
	register_pwm_irq_callback(onboard_led_control);
	configure_uart();
	configure_pushbutton();
	configure_led();
	configure_pwm();
	enable_irq();
	while(1)
	{
		asm("WFI");
		if(!systick_has_fired())
			continue;
		feed_the_watchdog();
	}
	return 0;
}

void onboard_led_control()
{
	static _Bool use_onboard_led = true;
	if (get_pb_pressed_flag())
		use_onboard_led = !use_onboard_led;
	if (use_onboard_led)
		toggle_led();
	else
		turn_off_led();
}

void led_pwm_control_fsm()
{
	static enum {WAIT_TX_RX, TX_COMMAND, RX_COMMAND, SET_PWM_DUTY_CYCLE, SET_PWM_FREQUENCY } state = WAIT_TX_RX;
	static char usb_c;
	static char uart_c;
	switch( state )
	{
		case WAIT_TX_RX:
			if (usbcdc_getchar(&usb_c))
				state = TX_COMMAND;
			else if (uart0_getchar(&uart_c))
				state = RX_COMMAND;
			break;
		case TX_COMMAND:
			if((usb_c == '1') | (usb_c == '2') | (usb_c == '3'))
			{
				uart0_putchar(usb_c);
				print_string("Transmit Duty Cycle Command\n\r");
			}
			if((usb_c == 'f') | (usb_c == 's'))
			{
				uart0_putchar(usb_c);
				print_string("Transmit Frequency Command\n\r");
			}
			state = WAIT_TX_RX;
			break;
		case RX_COMMAND:
			if((uart_c == '1') | (uart_c == '2') | (uart_c == '3'))
				state = SET_PWM_DUTY_CYCLE;
			else if((uart_c == 'f') | (uart_c == 's'))
				state = SET_PWM_FREQUENCY;
			else
				state = WAIT_TX_RX;
			break;
		case SET_PWM_DUTY_CYCLE:
			print_string("Changing Duty Cycle...\n\r");
			switch(uart_c)
			{
				case '1':
					set_pwm_duty_cycle(0xF000);
					break;
				case '2':
					set_pwm_duty_cycle(0xA000);
					break;
				case '3':
					set_pwm_duty_cycle(0x0400);
					break;
			}
			state = WAIT_TX_RX;
			break;
		case SET_PWM_FREQUENCY:
			print_string("Changing Frequency...\n\r");
			switch(uart_c)
			{
				case 'f':
					set_pwm_interval(0x0F<<4);
					break;
				case 's':
					set_pwm_interval(0xFF<<4);
					break;
			}
			state = WAIT_TX_RX;
			break;
	}
}
