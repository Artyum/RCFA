/*
 * f3a_tx.h
 *
 *  Created on: 19 mar 2015
 *      Author: Dexter
 */

#ifndef _F3A_TX_H_
#define _F3A_TX_H_

#include "../config.h"
#include "rcfa.h"
#if RADIO_MODE==1

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <ctype.h>

#include "../BTN/btn.h"
#include "../GPS/gps.h"
#include "../UART/uart.h"
#include "../RADIO/radio_tx.h"
#include "../KALMAN/kalman.h"
#include "../SD/sd.h"
#include "../MPL3115A2/mpl3115a2.h"
#include "../SPI/spi.h"
#include "../CALC/calc.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

extern volatile uint16_t t_vario;

typedef struct {
	double t_sub[VARIO_MAX_AVG_CNT];
	double prev_avg;
} s_vario;

//Ustawienia pocz¹tkowe strefy
void rcfa_init();

//Obliczenie wspó³rzêdnych punktów S, A i B
uint8_t calc_SAB();

//Okreœlenie po³o¿enia samolotu wzglêdem strefy - zwraca liczbê z zakresu -10000 +10000
int16_t check_position();

void led_blink();
void tx_loop();

#endif /* _F3A_TX_H_ */
#endif
