/*
 * sound.c
 *
 *  Created on: 11 wrz 2015
 *      Author: awitczak
 */

#ifndef SOUND_SOUND_C_
#define SOUND_SOUND_C_

#include "../config.h"
#if RADIO_MODE==0

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include "sound.h"
#include "../UART/uart.h"
#include "../RCFA/rcfa_rx.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

//Tryb buzzera
s_sound snd;
volatile uint16_t t_buzzer;

//Tablice sampli
volatile uint16_t tab_samples0[MAX_SAMPLES];
volatile uint16_t tab_samples1[MAX_SAMPLES];

//WskaŸnik do aktualnie granej tablicy
volatile uint16_t *tab_ready;

//Czêstotliwoœci w tablicach 0 i 1
double freq0, freq1;
uint8_t vol0, vol1;

void sound_init() {
	#if 0
	//Konfiguracja Timer1 16bit PWM P&F
	TCCR1A = (1<<COM1A1);				//Clear OC1A
	TCCR1B = (1<<WGM13)|(1<<CS10);		//PWM P&F Top:ICR1 | 1 presc.
	//TCCR1B = (1<<WGM13)|(1<<CS11);	//PWM P&F Top:ICR1 | 8 presc.
	ICR1 = 65000;
	OCR1A = 0;
	#else
	//Fast PWM
	TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);	//Tryb 14 (FAST PWM, TOP=ICR1)
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS10);		//Brak preskalera
	#endif

	//Wyjœcie sygna³u SIN na OC1A
	BIT_HIGH(BUZZER_DDR,BUZZER_PIN);

	ICR1 = TOP_SAMPLE;
	OCR1A = 5;
	//TIMSK1 = 0;

	tab_samples0[MAX_SAMPLES-1] = STOP_SAMPLE;
	tab_samples1[MAX_SAMPLES-1] = STOP_SAMPLE;

	snd.lp = 0;
	snd.vol = 100;
	snd.off = 1;
	snd.type = 0;
}

void sound_off() {
	//Wy³¹czenie przerwania Timer1
	if (snd.off) TIMSK1 = 0;
	//OCR1A = 0;
}

void chg_volume(int8_t c) {
	if (((snd.vol==VOLUME_MIN) && (c<0)) || ((snd.vol==VOLUME_MAX) && (c>0))) return;

	if (-c>snd.vol) {
		snd.vol=VOLUME_MIN;
	}
	else {
		snd.vol+=c;
		if (snd.vol>VOLUME_MAX) {
			snd.vol=VOLUME_MAX;
		}
	}
	//uart_puts("V:"); uart_putd(snd.vol,1); uart_nl();

	play_pause(50);
	delay_ms(50);
	play_freq(FE6, 50, snd.vol);
	delay_ms(100);
}

//Generacja tablicy sampli
void play_freq(double freq, uint16_t time, uint8_t vol) {
	uint8_t ready=0;

	if (freq<FREQ_MIN) freq=FREQ_MIN;
	if (freq>FREQ_MAX) freq=FREQ_MAX;
	if (vol>100) vol=100;

	//Jeœli czêstotliwoœæ jest w jednej z tablic to skok do gotowej tablicy
	if ((freq==freq0) && (vol==vol0)) { tab_ready=(uint16_t *)tab_samples0; vol0=vol; ready=1; }
	else if ((freq==freq1) && (vol==vol1)) { tab_ready=(uint16_t *)tab_samples1; vol1=vol; ready=1; }

	if (!ready) {
		uint16_t *n = (uint16_t *)tab_ready;
		uint16_t *ptr;

		//Generacja tablicy, która nie jest aktualnie odtwarzana
		if (n==tab_samples1) { ptr = (uint16_t *)tab_samples0; freq0 = freq; }
		else { ptr = (uint16_t *)tab_samples1; freq1 = freq; }
		n = ptr;

		double samples = (double)SAMPLE_RATE / freq;
		double v = 0.99*(vol/100.0)*TOP_SAMPLE/2;

		for (uint16_t i=0; i<samples; i++) {
			double rad = i/samples*PI_M2;
			double val = v*(cos(rad)+1);
			//double val = v*(cos(rad)+1+sin(rad*4)/15);
			//double val = v*(sin(rad)+1+sin(rad*4)/20);

			if (val>=v) *ptr=v;
			else if (val<=0) *ptr=0;
			else *ptr=val;

			//uart_putul(i,10); uart_puts(". "); uart_putul(*ptr,10); uart_nl();
			ptr++;
		}
		*ptr = STOP_SAMPLE;
		tab_ready = n;
	}

	t_buzzer = time;
	TIMSK1 = (1<<TOIE1);
}

void play_pause(uint16_t time) {
	sound_off();
	t_buzzer = time;
}

/*void play_key(uint8_t key, uint16_t time, double vol) {
	double freq = pow(HALF_TONE, key-49.0) * 440.0;
	play_freq(freq, time, vol);
}*/

#if FUNC_PLAY_TONE==1
void play_tone(uint8_t okt, uint8_t tone, uint16_t time, double vol) {
	double key = okt*12-8+tone;
	double freq = pow(HALF_TONE, key-49.0) * 440.0;
	play_freq(freq,time,vol);
}
#endif

//Generacja PWM Sinus
ISR (TIMER1_OVF_vect) {
	static uint16_t *p_tab=(uint16_t *)tab_samples0;

	if (*p_tab==STOP_SAMPLE) p_tab=(uint16_t *)tab_ready;
	OCR1A = *p_tab;
	p_tab++;
}

#endif
#endif /* SOUND_SOUND_C_ */
