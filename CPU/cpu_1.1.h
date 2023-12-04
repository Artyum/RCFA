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

#ifndef _CPU_H_
#define _CPU_H_

#include "../config.h"
#include <avr/io.h>
#include "../RADIO/radio.h"

#if PCB_REVISION==1

#if CPU==0

//BUTTON
#define BTN1_DDR		DDRC
#define BTN1_PORT		PORTC
#define BTN1_PINPORT	PINC
#define BTN1_PIN		PC3
#define BTN2_DDR		DDRC
#define BTN2_PORT		PORTC
#define BTN2_PINPORT	PINC
#define BTN2_PIN		PC1

//BUZZER
#define BUZZER_DDR		DDRB
#define BUZZER_PORT		PORTB
#define BUZZER_PIN		PB1

//LED
#define LED1_DDR		DDRC
#define LED1_PORT		PORTC
#define LED1_PIN		PC0
#define LED2_DDR		DDRC
#define LED2_PORT		PORTC
#define LED2_PIN		PC2

//Radio interrupt
#define INT_DDR			DDRD
#define INT_PORT		PORTD
#define INT_PINPORT		PIND
#define INT_FNE_PIN		PD7
#define INT_CRC_PIN		PD6
#define INT_FIFO_NOT_EMPTY	(INT_PINPORT & (1<<INT_FNE_PIN))
#define INT_CRCOK			(INT_PINPORT & (1<<INT_CRC_PIN))

//Radio frequency switch
#define FQ_DDR			DDRD
#define FQ_PORT			PORTD
#define FQ_PINPORT		PIND
#define FQ_PIN_1		PD3
#define FQ_PIN_2		PD2

//SPI
#define SPI_DDR			DDRB
#define SPI_PORT		PORTB
//#define SPI_SS_PORT		PORTB
//#define SPI_SS			PB2
#define SPI_SS_ORG		PB2
#define SPI_MOSI		PB3
#define SPI_MISO		PB4
#define SPI_SCK			PB5

#define SPI_SS_DDR		DDRC
#define SPI_SS_PORT		PORTC
#define SPI_SS			PC4

#define TIMER_VECT		TIMER0_COMPA_vect

#define LED1_ON			BIT_HIGH(LED1_PORT,LED1_PIN)
#define LED1_OFF		BIT_LOW(LED1_PORT,LED1_PIN)

#define LED2_ON			BIT_HIGH(LED2_PORT,LED2_PIN)
#define LED2_OFF		BIT_LOW(LED2_PORT,LED2_PIN)

#elif CPU==1

//Button
#define BTN1_DDR		DDRA
#define BTN1_PORT		PORTA
#define BTN1_PINPORT	PINA
#define BTN1_PIN		PA0

#define BTN2_DDR		DDRA
#define BTN2_PORT		PORTA
#define BTN2_PINPORT	PINA
#define BTN2_PIN		PA1

//LED
#define LED1_DDR		DDRA
#define LED1_PORT		PORTA
#define LED1_PIN		PA4

#define LED2_DDR		DDRA
#define LED2_PORT		PORTA
#define LED2_PIN		PA5

//Radio interrupt
#define INT_DDR			DDRD
#define INT_PORT		PORTD
#define INT_PINPORT		PIND
#define INT_PS_PIN		PD4
#define INT_FNE_PIN		PD5
#define INT_MR_PIN		PD6
#define INT_PACKET_SENT		(INT_PINPORT & (1<<INT_PS_PIN))
#define INT_FIFO_NOT_EMPTY	(INT_PINPORT & (1<<INT_FNE_PIN))
#define INT_CRCOK			(INT_PINPORT & (1<<INT_CRC_PIN))

//SPI Radio
#define SPI_DDR			DDRB
#define SPI_PORT		PORTB
#define SPI_MOSI		PB5
#define SPI_MISO		PB6
#define SPI_SCK			PB7
#define SPI_SS_ORG		PB4

//SPI frequency for the radio
//#define RADIO_FCLK	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)				/* F_CPU / 16 */
//#define RADIO_FCLK	SPCR = (1<<SPE)|(1<<MSTR)						/* F_CPU / 4 */

//SS
#define SPI_SS_DDR		DDRC
#define SPI_SS_PORT		PORTC
#define SPI_SS			PC2		/* SPI -> Radio */
#define SPI_SS_SD		PC3		/* SPI -> SD */

//Karta SD
#define SD_PORT			PORTD
#define SD_PIN			PIND
#define SD_CD			PD2		/* Card Detected */
//#define SD_WP			PB1		/* Write protected */

//Radio frequency switch
#define FQ_DDR			DDRB
#define FQ_PORT			PORTB
#define FQ_PINPORT		PINB
#define FQ_PIN_1		PB2
#define FQ_PIN_2		PB3

#define TIMER_VECT		TIMER0_COMPA_vect

#define SD_CS_LOW		SPI_SS_PORT &= ~(1<<SPI_SS_SD)		/* CS=low */
#define SD_CS_HIGH		SPI_SS_PORT |= (1<<SPI_SS_SD)		/* CS=high */
//#define SD_PWR_LOW	SD_PORT &= ~(1<<SD_PWR)
//#define SD_PWR_HIGH	SD_PORT |= (1<<SD_PWR)

//Card detected.   yes:true, no:false, default:true
#define SD_MMC_CD		(!(SD_PIN & (1<<SD_CD)))

//Write protected. yes:true, no:false, default:false
//#define SD_MMC_WP		(SD_PIN & (1<<SD_WP))
#define SD_MMC_WP		0

//SPI frequency for SD
//#define SD_FCLK_SLOW	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0)	/* F_CPU / 128 */
//#define SD_FCLK_SLOW	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)				/* F_CPU / 64 */
//#define SD_FCLK_SLOW	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)				/* F_CPU / 16 */

//#define SD_FCLK_FAST	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)				/* F_CPU / 128 */
//#define SD_FCLK_FAST	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)				/* F_CPU / 64 */
//#define SD_FCLK_FAST	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)				/* F_CPU / 16 */
//#define SD_FCLK_FAST	SPCR = (1<<SPE)|(1<<MSTR)						/* F_CPU / 4 */

#define LED1_ON			BIT_LOW(LED1_PORT,LED1_PIN)
#define LED1_OFF		BIT_HIGH(LED1_PORT,LED1_PIN)

#define LED2_ON			BIT_LOW(LED2_PORT,LED2_PIN)
#define LED2_OFF		BIT_HIGH(LED2_PORT,LED2_PIN)

#endif

// ************************************************************************************************************* //
// Macros and definitions

#define BIT_LOW(PORT,PIN)		PORT &= ~(1<<PIN)
#define BIT_HIGH(PORT,PIN)		PORT |= (1<<PIN)
#define BIT_TOG(PORT,PIN)		PORT ^= (1<<PIN)
//Returns 1 if bit == 1; 0 if bit == 0
#define READ_BIT(PINPORT,PIN)	(PINPORT & (1<<PIN))

#define LED1_TOG		BIT_TOG(LED1_PORT,LED1_PIN)
#define LED2_TOG		BIT_TOG(LED2_PORT,LED2_PIN)

//The number of elements in the array
#define NUM_ELEMENTS(x)	(sizeof(x)/sizeof(x[0]))

extern volatile uint16_t t_led1;
extern volatile uint16_t t_led2;
extern uint8_t led1_status;
extern uint8_t led2_status;

void cpu_init(void);
void delay_ms(uint16_t time);

#endif
#endif /* _CPU_H_ */
