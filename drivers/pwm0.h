// Description: Interface for driver for the PWM peripheral on the rp2040 microcontroller.

#ifndef PWM0_H
#define PWM0_H

#include <stdint.h>
#include <stdbool.h>

void configure_pwm(void);
void set_pwm_duty_cycle(uint16_t cc_value);
void set_pwm_interval(uint16_t clock_div);
_Bool register_pwm_irq_callback(void (*pf)());

#endif