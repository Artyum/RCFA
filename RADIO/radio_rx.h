/*
 * radio_rx.h
 *
 *  Created on: 8 mar 2015
 *      Author: Dexter
 */

#ifndef _RADIO_RX_H_
#define _RADIO_RX_H_

#include "../config.h"
#if RADIO_MODE==0

#include "../RCFA/rcfa.h"

#define RX_POS_WAIT		1010

void radio_init_rx();
void radio_rx_data();
//uint8_t radio_rx_coords(double *x, double *y);
//uint8_t radio_rx_byte(uint8_t *b);

void radio_hop_rx(int8_t num);

//extern uint8_t q[];

#endif
#endif /* _RADIO_RX_H_ */
