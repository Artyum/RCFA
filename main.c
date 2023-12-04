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

#include <avr/io.h>
#include "config.h"
#include "UART/uart.h"

#if PCB_REVISION==0
#include "CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "CPU/cpu_1.1.h"
#endif

#if RADIO_MODE==0
#include "RCFA/rcfa_rx.h"
#else
#include "RCFA/rcfa_tx.h"
#endif

int main(void) {
	cpu_init();

	//Put firmware version to screen
	uart_init(UBRR_38400);
	uart_puts(FIRMWARE_VER); uart_nl();

	//Delay for peripheral hardware initialization
	delay_ms(100);

#if RADIO_MODE==0

	//Receiver
	rx_loop();

#else

	//Transmitter
	uart_disable();
	tx_loop();

#endif
}
