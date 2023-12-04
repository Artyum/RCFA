/*
 * uart.h
 *
 *  Created on: 3 lut 2015
 *      Author: awitczak
 */

#ifndef _UART_H_
#define _UART_H_

#include <avr/io.h>
#include "../RADIO/radio.h"
#include "../config.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

#if CPU==0

#define USART_ENABLE_RX		UCSR0B |= (1<<RXCIE0)
#define USART_DISABLE_RX	UCSR0B &= ~(1<<RXCIE0)
#define UDRE_ON				UCSR0B |= (1<<UDRIE0)
#define UDRE_OFF			UCSR0B &= ~(1<<UDRIE0)
#define UART_UDR			UDR0

#define UART_RX_VECT		USART_RX_vect
#define UART_UDRE_VECT		USART_UDRE_vect

//Rozmiar buforów
#define BUFSIZE_TX		20
#define BUFSIZE_RX		10

#elif CPU==1

//#define USART_ENABLE_RX	UCSR0B |= (1<<RXCIE0)
//#define USART_DISABLE_RX	UCSR0B &= ~(1<<RXCIE0)
#define UDRE_ON				UCSR0B |= (1<<UDRIE0)
#define UDRE_OFF			UCSR0B &= ~(1<<UDRIE0)
#define UART_UDR			UDR0

#define UART_RX_VECT		USART0_RX_vect
#define UART_UDRE_VECT		USART0_UDRE_vect

//Rozmiar buforów
#define BUFSIZE_TX		30
#define BUFSIZE_RX		80	/* Zdanie NMEA ma max. ok. 71 znaków */

#endif

/*\
 *
 * Tabele prêdkoœci UART
 * http://wormfood.net/avrbaudcalc.php
 *
\*/

#if F_CPU==8000000

#define UBRR_4800		103
#define UBRR_9600		51
#define UBRR_14400		34
#define UBRR_19200		25
#define UBRR_38400		12
#define UBRR_57600		8
#define UBRR_115200		3

#elif F_CPU==10000000

#define UBRR_4800		129
#define UBRR_9600		64
#define UBRR_14400		42
#define UBRR_19200		32
#define UBRR_38400		15
#define UBRR_57600		10
#define UBRR_115200		4

#elif F_CPU==11059200

#define UBRR_4800		143
#define UBRR_9600		71
#define UBRR_14400		47
#define UBRR_19200		35
#define UBRR_38400		17
#define UBRR_57600		11
#define UBRR_115200		5

#elif F_CPU==12000000

#define UBRR_4800		155
#define UBRR_9600		77
#define UBRR_14400		51
#define UBRR_19200		38
#define UBRR_38400		19
#define UBRR_57600		12
#define UBRR_115200		6

#elif F_CPU==16000000

#define UBRR_4800		207
#define UBRR_9600		103
#define UBRR_14400		68
#define UBRR_19200		51
#define UBRR_38400		25
#define UBRR_57600		16
#define UBRR_115200		8

#endif

//#define MYUBRR ((F_CPU+UART_BAUD*8UL)/(16UL*UART_BAUD)-1)
//#define MYUBRR ((F_CPU/16UL/UART_BAUD)-1)

//Tryb pracy RX
//extern volatile uint8_t uart_rx_mode;

//Tablice danych z GPS
extern volatile char rmc_buf[];
extern volatile char gga_buf[];
extern volatile uint8_t rmc_ready;
extern volatile uint8_t gga_ready;

//W³aczenie UART
void uart_init(uint16_t ubrr);

//Wy³¹czenie UART
void uart_disable();

//Wys³anie znaku
void uart_putc(char data);

//Wysy³anie ci¹gu znaków
void uart_puts(const char *s);

//Wys³anie liczby ca³kowitej
void uart_putl(long int value, uint8_t radix);
void uart_putul(uint32_t value, uint8_t radix);

//Wys³anie liczby zmiennoprzecinkowej z podaniem liczby znaków po przecinku
#define DMAXSTRING 20
void uart_putd(double value, uint8_t prec);

#if RADIO_MODE==0
void uart_putsP(const char *p, uint8_t ret);
#endif

#if RADIO_MODE==1
//Wyœwietlenie c-stringa z EEPROM
void uart_pute(void *adr);
void uart_enable_rx();
void uart_disable_rx();
#endif

//Wyczyszczenie terminala
//void uart_clrscr();

//Enter \r\n
void uart_nl();

#endif /* _UART_H_ */
