/*
 * spi.h
 *
 *  Created on: 4 mar 2015
 *      Author: awitczak
 */

#ifndef _SPI_H_
#define _SPI_H_

#include <avr/io.h>
#include "../config.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

//void spi_tx(uint8_t Data);
//uint8_t spi_rx(void);

#define SPI_MODE_4		0x01
#define SPI_MODE_16		0x02
#define SPI_MODE_64		0x04
#define SPI_MODE_128	0x08
#define SPI_MODE_DR		0x80

//Liczba prób zapisu / odczytu
#define SPI_LOOP_CNT	200

void spi_enable(uint8_t mode);
void spi_disable();
void spi_write(uint8_t adr, uint8_t data);
uint8_t spi_read(uint8_t adr);
void spi_open();
void spi_close();

//void spi_tx(char Data);
//char spi_rx(void);

void spi_read_burst(uint8_t adr, uint8_t *tab, uint8_t cnt);
void spi_write_burst(uint8_t adr, uint8_t *tab, uint8_t cnt);

#endif /* _SPI_H_ */
