/*
 * mpl3115a2.h
 *
 *  Created on: 28 lip 2015
 *      Author: Dexter
 */

#ifndef MPL3115A2_MPL3115A2_H_
#define MPL3115A2_MPL3115A2_H_

#include "../config.h"
#if RADIO_MODE==1

#define MPL_ADR 0x60

#define MPL_STATUS_REG		0x00
#define MPL_OUT_P_MSB		0x01
#define MPL_OUT_P_CSB		0x02
#define MPL_OUT_P_LSB		0x03
#define MPL_OUT_T_MSB		0x04
#define MPL_OUT_T_LSB		0x05
#define MPL_DR_STATUS		0x06
#define MPL_WHO_AM_I		0x0C
#define MPL_PT_DATA_CFG		0x13
#define	MPL_CTRL_REG_1		0x26
#define	MPL_CTRL_REG_2		0x27
#define	MPL_CTRL_REG_3		0x28
#define	MPL_CTRL_REG_4		0x29
#define	MPL_CTRL_REG_5		0x2A

void mpl_init();
uint8_t mpl_getAlt(double *alt);

#endif
#endif /* MPL3115A2_MPL3115A2_H_ */
