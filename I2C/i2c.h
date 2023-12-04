/*
 * i2c_control.h
 *
 *	Obs³uga interfejsu I2C z uk³adem LSM9DS0
 *
 *  Created on: 8-9/2014
 *      Author: awitczak
 */

#ifndef I2C_CONTROL_H_
#define I2C_CONTROL_H_

#include "../config.h"
#if RADIO_MODE==1

#define I2C_READ 0x01
#define I2C_WRITE 0x00
#define I2C_TWINTSET (TWCR & (1<<TWINT))
#define I2C_RETTWSR (TWSR & 0xF8)
#define I2C_ILOOP 255

void i2c_init(void);
uint8_t i2c_start(void);
uint8_t i2c_write_data(uint8_t data);
uint8_t i2c_read_data_ack(uint8_t *data);
uint8_t i2c_read_data_nack(uint8_t *data);
uint8_t i2c_stop(void);

//uint8_t i2c_read_byte(uint8_t sla, uint8_t adr, uint8_t *data); //Do odczytu danych wykorzystywana jest funkcja i2c_read_sequence
uint8_t i2c_write_byte(uint8_t sla, uint8_t adr, uint8_t data);
uint8_t i2c_read_sequence(const uint8_t sla, const uint8_t adr, uint8_t dest[], const uint8_t cnt);

#endif
#endif /* I2C_CONTROL_H_ */
