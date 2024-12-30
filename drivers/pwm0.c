// Created by: Christopher Fehrer
// For: EECE 558, Final Project
// Description: Driver for the PWM peripheral on the rp2040 microcontroller. Has functions for 
//				controlling the duty-cycle and frequency of the PWM from outside the driver. Also 
//				has ability to register callbacks to the PWM IRQ.

#include "pwm0.h"
#include <rp2040/pwm.h>
#include <rp2040/resets.h>
#include <rp2040/sio.h>
#include <rp2040/io_bank0.h>
#include <rp2040/pads_bank0.h>
#include <rp2040/m0plus.h>
#include <stdint.h>
#include <stdbool.h>

#define PWM_GPIO 10

#define PWM_DIV_DEFAULT (0xFF<<4)
#define PWM_TOP 0xFFFF
#define PWM_CC_DEFAULT 0xA000;

#define PWM_IRQ_PRIO 3

#define CONCAT2(X,Y) X ## Y
#define CONCAT3(X,Y,Z) X ## Y ## Z
#define GPIO(X) CONCAT2(gpio,X)
#define GPIO_CTRL(X) CONCAT3(gpio,X,_ctrl)

#define PWM_RESETS ( RESETS_RESET_PWM_MASK \
				| RESETS_RESET_IO_BANK0_MASK \
				| RESETS_RESET_PADS_BANK0_MASK )

void __attribute__((isr)) pwm_irq_handler();

void configure_pwm(void)
{	
	resets -> clr_reset = PWM_RESETS;
	
	while( (resets->reset_done & PWM_RESETS) != PWM_RESETS);
	
	pads_bank0 -> GPIO(PWM_GPIO) = 
		PADS_BANK0_GPIO10_OD(0) 
		| PADS_BANK0_GPIO10_IE(0) 
		| PADS_BANK0_GPIO10_DRIVE(2) 
		| PADS_BANK0_GPIO10_PUE(1) 
		| PADS_BANK0_GPIO10_PDE(0) 
		| PADS_BANK0_GPIO10_SCHMITT(1) 
		| PADS_BANK0_GPIO10_SLEWFAST(0);
	
	io_bank0 -> GPIO_CTRL(PWM_GPIO) =
		IO_BANK0_GPIO10_CTRL_IRQOVER(0) |
		IO_BANK0_GPIO10_CTRL_INOVER(0)  |
		IO_BANK0_GPIO10_CTRL_OEOVER(0)  |
		IO_BANK0_GPIO10_CTRL_OUTOVER(0) |
		IO_BANK0_GPIO10_CTRL_FUNCSEL(4);
	
	
	(( void (**)()) m0plus->vtor )[16+4] = pwm_irq_handler;
	m0plus->nvic_ipr1 = (m0plus->nvic_ipr1 & ~M0PLUS_NVIC_IPR1_IP_4_MASK)
		| M0PLUS_NVIC_IPR1_IP_4(PWM_IRQ_PRIO);
	m0plus -> nvic_iser |= (1<<4);
	
	pwm->ch5_csr = PWM_CH5_CSR_A_INV_MASK | PWM_CH5_CSR_EN_MASK;
	pwm->ch5_div = PWM_DIV_DEFAULT;
	pwm->ch5_cc = PWM_CC_DEFAULT;
	pwm->ch5_top = PWM_TOP;
	pwm->inte = PWM_INTE_CH5_MASK;
	pwm->en = PWM_EN_CH5_MASK;
}

void set_pwm_duty_cycle(uint16_t cc_value)
{
	pwm->ch5_cc = cc_value;
}

void set_pwm_interval(uint16_t clock_div)
{
	pwm->ch5_div = clock_div;
}

#ifndef NUM_CALLBACKS
#define NUM_CALLBACKS 5
#endif

static void (*callback[NUM_CALLBACKS])();
static uint32_t num_callbacks;

_Bool register_pwm_irq_callback(void (*pf)())
{
	if (NUM_CALLBACKS == num_callbacks)
		return false;
	callback[num_callbacks++] = pf;
	return true;
}	

void __attribute__((isr)) pwm_irq_handler()
{	
	m0plus -> nvic_icpr = (1<<4);
	pwm -> intr = PWM_INTR_CH5_MASK;
	for(uint32_t i = 0; i < num_callbacks; i++)
		callback[i]();
}