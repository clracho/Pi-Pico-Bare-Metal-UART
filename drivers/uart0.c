// Description: Driver for the UART peripheral on the rp2040 processor. Utilizes interrupts for
//				receiving data. FIFOs disabled, single character transmission capability.
//				Also has full duplex capability.

#include "uart0.h"
#include "interrupt.h"
#include <rp2040/uart.h>
#include <rp2040/resets.h>
#include <rp2040/sio.h>
#include <rp2040/io_bank0.h>
#include <rp2040/pads_bank0.h>
#include <rp2040/m0plus.h>
#include <rp2040/clocks.h>
#include <stdint.h>
#include <stdbool.h>

#define UART_GPIO_TX 0
#define UART_GPIO_RX 1

#define BAUD_RATE_DIVISOR_I 217
#define BAUD_RATE_DIVISOR_F 4

#define UART0_IRQ_PRIO 3

#define CONCAT2(X,Y) X ## Y
#define CONCAT3(X,Y,Z) X ## Y ## Z
#define GPIO(X) CONCAT2(gpio,X)
#define GPIO_CTRL(X) CONCAT3(gpio,X,_ctrl)

#define UART_RESETS ( RESETS_RESET_UART0_MASK \
				| RESETS_RESET_IO_BANK0_MASK \
				| RESETS_RESET_PADS_BANK0_MASK )

void __attribute__((isr)) uart0_handler();

void configure_uart(void)
{	
	clocks -> clk_peri_ctrl = CLOCKS_CLK_PERI_CTRL_ENABLE_MASK;
	
	resets -> clr_reset = UART_RESETS;
	
	while( (resets->reset_done & UART_RESETS) != UART_RESETS);
	
	pads_bank0 -> GPIO(UART_GPIO_TX) = 
		PADS_BANK0_GPIO0_OD(0) 
		| PADS_BANK0_GPIO0_IE(0) 
		| PADS_BANK0_GPIO0_DRIVE(0) 
		| PADS_BANK0_GPIO0_PUE(0) 
		| PADS_BANK0_GPIO0_PDE(0) 
		| PADS_BANK0_GPIO0_SCHMITT(0) 
		| PADS_BANK0_GPIO0_SLEWFAST(0);
	
	pads_bank0 -> GPIO(UART_GPIO_RX) = 
		PADS_BANK0_GPIO1_OD(0) 
		| PADS_BANK0_GPIO1_IE(1) 
		| PADS_BANK0_GPIO1_DRIVE(0) 
		| PADS_BANK0_GPIO1_PUE(0) 
		| PADS_BANK0_GPIO1_PDE(0) 
		| PADS_BANK0_GPIO1_SCHMITT(1) 
		| PADS_BANK0_GPIO1_SLEWFAST(0);
	
	io_bank0 -> GPIO_CTRL(UART_GPIO_TX) =
		IO_BANK0_GPIO0_CTRL_IRQOVER(0) |
		IO_BANK0_GPIO0_CTRL_INOVER(0)  |
		IO_BANK0_GPIO0_CTRL_OEOVER(0)  |
		IO_BANK0_GPIO0_CTRL_OUTOVER(0) |
		IO_BANK0_GPIO0_CTRL_FUNCSEL(2);
	
	io_bank0 -> GPIO_CTRL(UART_GPIO_RX) =
		IO_BANK0_GPIO1_CTRL_IRQOVER(0) |
		IO_BANK0_GPIO1_CTRL_INOVER(0)  |
		IO_BANK0_GPIO1_CTRL_OEOVER(0)  |
		IO_BANK0_GPIO1_CTRL_OUTOVER(0) |
		IO_BANK0_GPIO1_CTRL_FUNCSEL(2);
	
	uart0 -> uartibrd = BAUD_RATE_DIVISOR_I;
	uart0 -> uartfbrd = BAUD_RATE_DIVISOR_F;
	uart0 -> uartlcr_h |= UART0_UARTLCR_H_WLEN_MASK;
	uart0 -> uartcr |= UART0_UARTCR_RXE_MASK | UART0_UARTCR_TXE_MASK | UART0_UARTCR_UARTEN_MASK;
	uart0 -> uartimsc = UART0_UARTIMSC_RXIM_MASK;
	
	(( void (**)()) m0plus->vtor )[16+20] = uart0_handler;
	m0plus -> nvic_ipr5 = (m0plus->nvic_ipr5 & ~M0PLUS_NVIC_IPR5_IP_20_MASK)
		| M0PLUS_NVIC_IPR5_IP_20(UART0_IRQ_PRIO);
	m0plus -> nvic_iser |= (1<<20);
}

static uint16_t rx_data;
static _Bool rx_data_received = false;

void uart0_putchar(char data)
{
	_Bool TX_fifo_full = (uart0 -> uartfr) & UART0_UARTFR_TXFF_MASK;
	if (!TX_fifo_full)
		uart0 -> uartdr = data;
}

_Bool uart0_getchar(char *data)
{
	
	disable_irq();
	_Bool retval;
	retval = rx_data_received;
	*data = rx_data;
	enable_irq();
	rx_data_received = false;
	return retval;
}

void __attribute__((isr)) uart0_handler()
{	
	m0plus -> nvic_icpr = (1<<20);
	rx_data_received = true;
	rx_data = uart0 -> uartdr;
}