/*
 * i2c_control.c
 *
 *	Obs³uga interfejsu I2C z uk³adem LSM9DS0
 *
 *  Created on: 8-9/2014
 *      Author: awitczak
 */

#include "../config.h"
#if RADIO_MODE==1

#include <avr/io.h>
#include "i2c.h"

// Function to initialize master
void i2c_init(void) {
	//Prescaler: 1
	//TWSR = (0<<TWPS1) | (0<<TWPS0);

	//TWBR = 18;	//100KHz Atmega644 16MHz
	TWBR = 4;		//400KHz Atmega644 16MHz
}

uint8_t i2c_start(void) {
	uint8_t i=I2C_ILOOP;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while ((i) && (!I2C_TWINTSET)) i--;
	if (I2C_TWINTSET) return I2C_RETTWSR; else return 0;
}

uint8_t i2c_write_data(uint8_t data) {
	uint8_t i=I2C_ILOOP;
	TWDR=data; // put data in TWDR
	TWCR = (1<<TWINT) | (1<<TWEN);
	while ((i) && (!I2C_TWINTSET)) i--;
	if (I2C_TWINTSET) return I2C_RETTWSR; else return 0;
}

uint8_t i2c_read_data_ack(uint8_t *data) {
	uint8_t i=I2C_ILOOP;
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while ((i) && (!I2C_TWINTSET)) i--;
	if (I2C_TWINTSET) {
		*data = TWDR;
		return I2C_RETTWSR;
	}
	else return 0;
}

uint8_t i2c_read_data_nack(uint8_t *data) {
	uint8_t i=I2C_ILOOP;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while ((i) && (!I2C_TWINTSET)) i--;
	if (I2C_TWINTSET) {
		*data = TWDR;
		return I2C_RETTWSR;
	}
	else return 0;
}

uint8_t i2c_stop(void) {
	uint8_t i=I2C_ILOOP;
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	while ((i) && (!I2C_TWINTSET)) i--;
	if (I2C_TWINTSET) return I2C_RETTWSR; else return 0;
}

/*uint8_t i2c_read_byte(uint8_t sla, uint8_t adr, uint8_t *data) {
	//Start
	if (i2c_start() != 0x08) { i2c_stop(); return 0; }

	//Wys³anie adresu urz¹dzenia SLAVE + WRITE
	if (i2c_write_data((sla<<1) | I2C_WRITE) != 0x18) { i2c_stop(); return 0; }

	//Wys³anie subadresu
	if (i2c_write_data(adr) != 0x28 ) { i2c_stop(); return 0; }

	//Repeated Start
	if (i2c_start() != 0x10) { i2c_stop(); return 0; }

	//Wys³anie adresu urz¹dzenia SLAVE + READ
	if (i2c_write_data((sla<<1) | I2C_READ) != 0x40) { i2c_stop(); return 0; }

	//Odczyt danych
	if (i2c_read_data_nack(data) != 0x58) { i2c_stop(); return 0; }

	i2c_stop();
	return 1;
}*/

uint8_t i2c_write_byte(const uint8_t sla, const uint8_t adr, uint8_t data) {
	//Start
	if (i2c_start() != 0x08) { i2c_stop(); return 0; }

	//Wys³anie adresu urz¹dzenia SLAVE + WRITE
	if (i2c_write_data((sla<<1) | I2C_WRITE) != 0x18) { i2c_stop(); return 0; }

	//Wys³anie subadresu
	if (i2c_write_data(adr) != 0x28 ) { i2c_stop(); return 0; }

	//Wys³anie bajtu
	if(i2c_write_data(data) != 0x28 ) { i2c_stop(); return 0; }

	i2c_stop();
	return 1;
}

uint8_t i2c_read_sequence(const uint8_t sla, const uint8_t adr, uint8_t dest[], const uint8_t cnt) {
	//Start
	if (i2c_start() != 0x08) { i2c_stop(); return 0; }

	//Wys³anie adresu urz¹dzenia SLAVE + WRITE
	if (i2c_write_data((sla<<1) | I2C_WRITE) != 0x18) { i2c_stop(); return 0; }

	//Wys³anie subadresu + autoincrement
	//if (i2c_write_data(adr | (1<<7)) != 0x28) { i2c_stop(); return 0; }
	if (i2c_write_data(adr) != 0x28) { i2c_stop(); return 0; }

	//Repeated Start
	if (i2c_start() != 0x10) { i2c_stop(); return 0; }

	//Wys³anie adresu urz¹dzenia SLAVE + READ
	if (i2c_write_data((sla<<1) | I2C_READ) != 0x40) { i2c_stop(); return 0; }

	//Odczyt danych
	for (uint8_t i=0; i<(cnt-1); i++) { if (i2c_read_data_ack(&dest[i]) != 0x50) { i2c_stop(); return 0; }}		//Odczyt z ACK
	if (i2c_read_data_nack(&dest[cnt-1]) != 0x58) { i2c_stop(); return 0; }										//Odczyt ostatniego bajtu z NACK

	i2c_stop();
	return 1;
}

#endif
