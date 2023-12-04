/*
 * This file is part of RC Flight Assist (RCFA).
 *
 * RCFA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RCFA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Created by Arek "Artyum" Witczak for F3A pattern flying trening
 * Developed since 2014/08
 *
 * Project HomePage: http://www.littlecircuit.com/
 *
 */

#ifndef _BTN_H_
#define _BTN_H_

#include <avr/io.h>

/* Status of the buttons
 *
 * KEY_UP			- Button not pressed
 * KEY_DN_SHORT_S	- Moment of pressing the button - a short click
 * KEY_DN_SHORT_C	- Button pressed - short click
 * KEY_DN_LONG_S	- The moment you start a long click
 * KEY_DN_LONG_C	- Button pressed - long click
 * KEY_UP_SHORT		- The button is released after a short click
 * KEY_UP_LONG		- Button released after a long click
 *
 */

#define KEY_UP				0
#define KEY_DN_SHORT_S		2
#define KEY_DN_SHORT_C		3
#define KEY_DN_LONG_S		4
#define KEY_DN_LONG_C		5
#define KEY_UP_SHORT		11
#define KEY_UP_LONG			14

#define KEYPRESS_SHORT		20
#define KEYPRESS_LONG		700

//Macro 0-button released; 1-button pressed
#define BTN_PRESSED(PINPORT,PIN) (!(PINPORT & (1<<PIN)))

extern volatile uint16_t btn1_timer;
extern volatile uint16_t btn2_timer;
extern uint8_t btn1_status;
extern uint8_t btn2_status;

//Asynchronous button operation
void key_check(uint8_t *status, volatile uint16_t *timer, uint8_t pinport, uint8_t pin);

#endif /* _BTN_H_ */
