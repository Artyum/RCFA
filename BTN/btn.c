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

#include "btn.h"

//Z point set / Flight Area lock
volatile uint16_t btn1_timer;

//P point set / Logging Start and Stop
volatile uint16_t btn2_timer;

uint8_t btn1_status;
uint8_t btn2_status;

// Key support - short and long press
void key_check(uint8_t *status, volatile uint16_t *timer, uint8_t pinport, uint8_t pin) {

	//Button pressed
	if (BTN_PRESSED(pinport,pin)) {
		if (*status==0)	{ *status=1; *timer=0; } //1 - Initial pressing
		else if (*status==1 && *timer>=KEYPRESS_SHORT) { *status=2; *timer=0; } //2 - Short press start
		else if (*status==2) *status=3; //3 - Short press continuous
		else if (*status==3 && *timer>=KEYPRESS_LONG) { *status=4; *timer=0; } //4 - Long press start
		else if (*status==4) *status=5; //5 - Long press continuous
	}

	//Button released
	else if (*status!=0) {
		if (*status==1) *status=0;
		else if (*status==2 || *status==3) { *status=11; *timer=0; } //11 - End short press
		else if (*status==11) *status=0; //Exit short press
		else if (*status==4 || *status==5) { *status=14; *timer=0; } //14 - End long press
		else if (*status==14) *status=0; //End long press
	}

}
