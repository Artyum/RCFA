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
#include <avr/interrupt.h>

#include "../config.h"
#include "../RADIO/radio.h"
#include "../RADIO/radio_tx.h"
#include "../RADIO/radio_rx.h"
#include "../BTN/btn.h"
#include "../SD/diskio.h"
#include "../SD/sd.h"
#include "../GPS/gps.h"
#include "../SPI/spi.h"
#include "../KALMAN/kalman.h"
#include "../RCFA/rcfa_rx.h"
#include "../RCFA/rcfa_tx.h"
#include "../SOUND/sound.h"

#if PCB_REVISION==0
#include "cpu_1.0.h"
#elif PCB_REVISION==1
#include "cpu_1.1.h"
#endif

volatile uint16_t t_delay;

//Zmienne dla LED
volatile uint16_t t_led1;
volatile uint16_t t_led2;
uint8_t led1_status;
uint8_t led2_status;

void cpu_init(void) {

#if CPU==0
	//Atmega168P @ 11.0592MHz

	//Timer0 8-bit configuration
	TCCR0A = (1<<WGM01);	//CTC
	TCCR0B = (1<<CS02);		//256 prescaler
	#if F_CPU==8000000
	OCR0A = 31;				//Interrupt every 1ms @ 8MHz
	#elif F_CPU==10000000
	OCR0A = 39;				//Interrupt every 1ms @ 10MHz
	#elif F_CPU==11059200
	OCR0A = 43;				//Interrupt every 1ms @ 11.059MHz
	#elif F_CPU==12000000
	OCR0A = 47;
	#endif

	//Timer0 Compare Match A
	TIMSK0 |= (1<<OCIE0A);

	//IN CRC_OK
	BIT_HIGH(INT_PORT,INT_CRC_PIN);

	//IN FIFO_NOT_EMPTY
	BIT_HIGH(INT_PORT,INT_FNE_PIN);


#elif CPU==1
	//Atmega644 @ 16MHz

	//Timer0 8-bit configuration
	TCCR0A = (1<<WGM01);			//CTC Top OCRA
	TCCR0B = (1<<CS01)|(1<<CS00);	//64 prescaler
	OCR0A = 250;  //Interrupt every 1ms

	//Compare Match na Timer0
	TIMSK0 = (1<<OCIE0A);

	//IN PACKET_SENT
	BIT_HIGH(INT_PORT,INT_PS_PIN);

	//Card Detected pin
	BIT_HIGH(SD_PORT,SD_CD);

	//IN FIFO_NOT_EMPTY
	//BIT_HIGH(INT_PORT,INT_FNE_PIN);

	//IN MODE_READY
	//BIT_HIGH(INT_PORT,INT_MR_PIN);

	//IN Przerwania
	//INT_DDR &= ~((1<<INT_FNE)|(1<<INT_MR)|(1<<INT_PS));
	//INT_PORT &= ~((1<<INT_FNE)|(1<<INT_MR)|(1<<INT_PS));

#endif

	//Radio frequency selector
	BIT_HIGH(FQ_PORT, FQ_PIN_1);
	BIT_HIGH(FQ_PORT, FQ_PIN_2);

	//IN BTN1, BTN2 (DDR low)
	BIT_HIGH(BTN1_PORT, BTN1_PIN);
	BIT_HIGH(BTN2_PORT, BTN2_PIN);

	//OUT LED1, LED2
	BIT_HIGH(LED1_DDR,LED1_PIN); LED1_OFF;
	BIT_HIGH(LED2_DDR,LED2_PIN); LED2_OFF;

	sei();
}

void delay_ms(uint16_t time) {
	uint8_t i;
	t_delay = time;
	do {
		i = 100;
		while (--i) { asm ("nop"); asm ("nop"); }
	} while (t_delay>0);
}

//Interrupt run every 1 ms - software timers
ISR (TIMER_VECT) {
	uint16_t n;

	n = t_led1; if (n) t_led1 = --n;
	n = t_led2; if (n) t_led2 = --n;
	n = t_delay; if (n) t_delay = --n;

	#if RADIO_MODE==0
	//Receiver
	n = t_buzzer; if (n) { t_buzzer = --n; if (n==1) sound_off(); }
	n = t_rx; if (n) t_rx = --n;
	n = t_nmea; if (n) t_nmea = --n;
	n = t_rxpos; if (n) t_rxpos = --n;

	#else

	//Transmitter
	n = t_radio_tx; if (n) t_radio_tx = --n;
	n = t_getGPSdata; if (n) t_getGPSdata = --n;
	n = t_gpsBreak; if (n) t_gpsBreak = --n;
	n = t_vario; if (n) t_vario = --n;

	//SD
	n = t_logger_buf; if (n) t_logger_buf = --n;
	n = t_sd_proc; if (n) t_sd_proc = --n;
	else {
		t_sd_proc=10;
		disk_timerproc();
	}

	#endif

	//Buttons timers
	btn1_timer++;
	btn2_timer++;
}
