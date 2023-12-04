/*
 * mpl3115a2.c
 *
 *  Created on: 28 lip 2015
 *      Author: Dexter
 */

#include "../config.h"
#if RADIO_MODE==1

#include <avr/io.h>
#include "mpl3115a2.h"
#include "../I2C/i2c.h"
#include "../RCFA/rcfa.h"

void mpl_init() {
	uint8_t ret[1];

	//Inicjalizacja I2C
	i2c_init();

	ret[0]=0;

	if (i2c_read_sequence(MPL_ADR, MPL_WHO_AM_I, ret, 1)) {
		if (ret[0]==0xC4) {

			i2c_write_byte(MPL_ADR, MPL_PT_DATA_CFG, 0b00000111);	//Generate data ready
			//i2c_write_byte(MPL_ADR, MPL_CTRL_REG_1, 0b10100001);	//Altimeter mode | Oversample Ratio 16 (66ms) | Reset disabled | Set ACTIVE
			i2c_write_byte(MPL_ADR, MPL_CTRL_REG_1, 0b10011001);	//Altimeter mode | Oversample Ratio 8 (34ms)  | Reset disabled | Set ACTIVE
			//i2c_write_byte(MPL_ADR, MPL_CTRL_REG_1, 0b10010001);	//Altimeter mode | Oversample Ratio 4 (18ms)  | Reset disabled | Set ACTIVE

			hw.mpl = 1;
		}
	}
}

uint8_t mpl_getAlt(double *alt) {
	uint8_t data[3];
	data[0]=0;

	//Ustawienie bitu OST
	i2c_write_byte(MPL_ADR, MPL_CTRL_REG_1, 0b10010011);

	//Read status Reg
	if (!i2c_read_sequence(MPL_ADR, MPL_DR_STATUS, data, 1)) return 0;

	if (data[0] & 0x08) {
		//Odczyt nowych danych
		if (!i2c_read_sequence(MPL_ADR, MPL_OUT_P_MSB, data, 3)) return 0;

		int16_t meters = (data[0]<<8) | data[1];
		uint8_t fract = (data[2]>>4);

		*alt = (double)meters + (double)fract/16.0;

		//Wy³¹czenie bitu OST
		i2c_write_byte(MPL_ADR, MPL_CTRL_REG_1, 0b10010001);
		return 1;
	}
	return 0;
}

#endif
