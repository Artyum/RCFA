/*
 * kalman.h
 *
 *  Created on: 17 mar 2015
 *      Author: awitczak
 */

#ifndef _KALMAN_H_
#define _KALMAN_H_

#include "../config.h"
#if RADIO_MODE==1

typedef struct {
	double xk;
	double pk;

	//Znacznik wy¿arzania 1-gotowoœæ | 0-wy¿arzanie
	uint8_t ready;

	//Licznik cykli wy¿arzania
	//uint8_t cnt;
} s_kalman;

//Liczba cykli wy¿arzania
#define FILTER_PREP		200

#define C_READY_PREP	0.00005
#define ALT_READY_PREP	0.03

//R		-	Q
//1		-	0.1		1.7m
//0.1	-	0.1		ca³kiem OK
//0.1	-	0.5		s³abiej niz 0.1-0.1
//0.5	-	0.1		za mocno - dla wysokoœci mo¿e byæ mocniej

//#define KAL_R			0.5
//#define KAL_Q			0.1
//#define KAL_R_ALT		1.0
//#define KAL_Q_ALT		10.0

//Liczba próbek do wy¿arzenia filtra
//#define KAL_PREP_CNT	200

extern volatile uint8_t t_kalman;

//Zmienne do filtru kalmana
extern s_kalman klat;
extern s_kalman klon;
extern s_kalman kalt;
extern s_kalman kvario;

extern uint8_t kalman_prep;
double kalman(double zk, double *xk, double *Pk, double R, double Q);
uint8_t filter_ready(double a, double b, double wsp);
void filter_reset();

#endif
#endif /* _KALMAN_H_ */
