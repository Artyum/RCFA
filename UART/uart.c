/*
 * uart.c
 *
 *  Created on: 3 lut 2015
 *      Author: awitczak
 */

#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "../config.h"
#include "../RCFA/rcfa.h"
#include "../GPS/gps.h"
#include "../SD/sd.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

#if RADIO_MODE==1
volatile uint8_t rx_enabled;
#endif

void uart_init(uint16_t ubrr) {
	// Ustawienie prêdkoœci transmisji
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)ubrr;

	#if RADIO_MODE==0
	//W³¹czenie nadajnika
	UCSR0B = (1<<TXEN0);
	#else
	//W³¹czenie nadajnika i odbiornika + Przerwanie RX Complete Interrupt Enable
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	#endif

	// Ustawienie formatu ramki:
	// Asynchronous, 8 bitów danych, 1 bit stopu, brak parzystoœci
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

	#if RADIO_MODE==1
	rx_enabled = 1;
	#endif
}

void uart_disable() {
	UBRR0H = 0;
	UBRR0L = 0;
	UCSR0C = 0;
	UCSR0B = 0;
}

#if RADIO_MODE==1
void uart_disable_rx() {
	rx_enabled = 0;
	//UCSR0B &= ~(1<<RXCIE0);
}

void uart_enable_rx() {
	rmc_ready = 0;
	gga_ready = 0;
	strcpy((char *)rmc_buf, "");
	strcpy((char *)gga_buf, "");
	t_gpsBreak = logger.gpsBreak_time;
	//UCSR0B |= (1<<RXCIE0);
	rx_enabled = 1;
}
#endif

// Wysy³anie /////////////////////////////////////

//Bufor danych TX
volatile char uart_tx_buf[BUFSIZE_TX];
volatile uint8_t uart_txi_w;	//Index w buforze TX do zapisu do bufora
volatile uint8_t uart_txi_r;	//Index w buforze TX do odczytu z bufora
volatile uint8_t uart_tx_drop;	//WskaŸnik utraconych pakietów

//Wstawienie znaku do bufora TX
void uart_putc(char data) {
	uint8_t tmp;

	//Sprawdzenie czy jest miejsce w buforze - czy indeks zapisu "dogoni" indeks odczytu
	tmp = uart_txi_w + 1;
	if (tmp>=BUFSIZE_TX) tmp = 0;

	//Oczekiwanie na miejsce w buforze
	//while (tmp==uart_txi_r);
	while (tmp==uart_txi_r) for (uint8_t i=0; i<50; i++) { asm("nop"); asm("nop"); }

	uart_txi_w = tmp;
	uart_tx_buf[uart_txi_w] = data;

	//Zezwolenie na przerwanie na flagê UDRE=1 - UDR is ready to receive new data
	UDRE_ON;
}

//Wstawienie wielu znaków do bufora TX
void uart_puts(const char *s) {
    register char c;
    while ((c=*s++)) uart_putc(c);
}

/*void uart_putsP(const char * p) {
	char c;
	do {
		c = pgm_read_byte(p++);
		if (c) uart_putc(c);
	} while (c);
}*/

//Wstawienie liczby do bufora TX
void uart_putl(long int value, uint8_t radix) {
	char string[DMAXSTRING];
	ltoa(value, string, radix);
	uart_puts(string);
}

//Wstawienie liczby do bufora TX
void uart_putul(uint32_t value, uint8_t radix) {
	char string[DMAXSTRING];
	ultoa(value, string, radix);
	uart_puts(string);
}

void uart_putd(double value, uint8_t prec) {
	char string[DMAXSTRING+1];
	dtostrf(value,1,prec,string);
	uart_puts(string);
}

#if RADIO_MODE==1
void uart_pute(void *adr) {
	register char c;
	uint8_t *ptr = (uint8_t *)adr;
	while ((c=eeprom_read_byte(ptr++))) uart_putc(c);
}
#endif

/*void uart_clrscr() {
	uart_putc(27);
	uart_puts("[2J");
	uart_putc(27);
	uart_puts("[H");
}*/

void uart_nl() {
	uart_puts("\r\n");
}

#if RADIO_MODE==0
//Wyœwietlenie tekstu z pamiêci flash
void uart_putsP(const char *p, uint8_t ret) {
	register char c;
	while ((c=pgm_read_byte(p++))) uart_putc(c);
	if (ret) uart_nl();
}
#endif

ISR (UART_UDRE_VECT) {
	if (uart_txi_r != uart_txi_w) {
		uart_txi_r++;
		if (uart_txi_r>=BUFSIZE_TX) uart_txi_r=0;
		UART_UDR = uart_tx_buf[uart_txi_r];
	}
	else {
		//Wy³¹czenie przerwania jeœli bufor pusty
		UDRE_OFF;
	}
}

#if RADIO_MODE==1
// Odbieranie /////////////////////////////////////

//Bufory danych RX
volatile char gga_buf[BUFSIZE_RX+1];
volatile char rmc_buf[BUFSIZE_RX+1];
volatile uint8_t gga_ready; //0-trwa odczyt; >0-odczyt zakoñczony, zmienna zawiera liczbê znaków w linni
volatile uint8_t rmc_ready; //0-trwa odczyt; >0-odczyt zakoñczony, zmienna zawiera liczbê znaków w linni

ISR (UART_RX_VECT) {
	static uint8_t i;
	char data = UART_UDR;

	if (rx_enabled==0) return;

	//Napotkano na nowy rekord
	if (data=='$') i = 0;

	//Przepe³nienie bufora
	if (i>=BUFSIZE_RX) {
		i = 0;
		gga_ready = 0;
		rmc_ready = 0;
	}

	//Odczyt GGA
	if (gga_ready==0) {
		gga_buf[i] = data;
		if ((i>6) && (gga_buf[i-2]=='*') && (gga_buf[3]=='G') && (gga_buf[4]=='G') && (gga_buf[5]=='A')) {
			gga_ready=i+1;
			gga_buf[i+1]='\0';
			i = 0;
		}
	}

	//Oczyt RMC
	if (rmc_ready==0) {
		rmc_buf[i] = data;
		if ((i>6) && (rmc_buf[i-2]=='*') && (rmc_buf[3]=='R') && (rmc_buf[4]='M') && (rmc_buf[5]=='C')) {
			rmc_ready=i+1;
			rmc_buf[i+1]='\0';
			i = 0;
		}
	}

	i++;
}
#endif
