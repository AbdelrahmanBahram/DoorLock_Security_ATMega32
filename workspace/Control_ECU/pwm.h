/*
 * pwm.h
 *
 * Author: Abdelrahman Bahram
 */

#ifndef PWM_H_
#define PWM_H_

#include <avr/io.h>  // Include AVR library for register definitions
#include"std_types.h"
#include"gpio.h"


void PWM_Timer0_Start(uint8 duty_cycle);

#endif /* PWM_H_ */
