/*
 * f3a.h
 *
 *  Created on: 5 mar 2015
 *      Author: awitczak
 */

#ifndef _F3A_RX_H_
#define _F3A_RX_H_

#include "../config.h"
#if RADIO_MODE==0

#include "../RCFA/rcfa.h"

//Tryby buzzera alarmu
#define BUZZ_ALARM_M		1
#define BUZZ_ALARM_T		2

//Rodzaj odebranych danych
#define SND_TYPE_ALARM		0x01	/* 0-nie alarm | 1 - alarm */
#define SND_TYPE_P1			0x02	/* 0-nie P1 | 1-P1 */
#define SND_TYPE_P2			0x04	/* 0-nie P2 | 1-P2 */
#define SND_TYPE_CONF		0x08
#define SND_TYPE_RTB		0x10
#define SND_TYPE_BLK		0x20
#define SND_TYPE_NOSIG		0x40
#define SND_TYPE_NOFIX		0x80
#define SND_TYPE_REFPOS		0x100
#define SND_TYPE_VALARM		0x200

//Znaczniki odebrania danych
#define RX_CONFIG_SET		0x01
#define RX_REFALT_SET		0x02
#define RX_REFPOS_SET		0x04
#define RX_RTB_SET			0x08
#define RX_BLK_SET			0x10

//Czasy "pikacza" dla alarmu
#define BUZZER_DELAY_MIN	80		/* Minimalna przerwa miêdzy sygna³ami */
#define BUZZER_DELAY_MAX	700		/* Maksymalna przerwa miêdzy sygna³ami */

//Czêstotliwoœci pikacza dla wario
#define VARIO_FREQ_MIN		SND_FREQ_MIN
#define VARIO_FREQ_MAX		(SND_FREQ_MIN+400)
#define VARIO_TIMER			333

#define VARIO_TIME_MIN		130
#define VARIO_TIME_MAX		600

//Czêstotliwoœæ generacji NMEA
#define NMEA_GEN_TIME		1000

//Blokada wielokrotnego odbierania danych [ms]
#define CNT_INFO_TIME		2000
#define CNT_ALARM_TIME		1500

//Tagi
#define TAG_CONF			0x01
#define TAG_READY			0x02
#define TAG_BLK				0x04
#define TAG_P1				0x08
#define TAG_P2				0x10

#define TAG_RESET_TIME		60000

#define DEG2LAT				1
#define DEG2LON				2

//Znaczniki diod
#define S_LED1				0x01
#define S_LED2				0x02

typedef struct {
	s_wsp w;
	s_wsp ref;
	double alt;
	double spd;
	uint16_t sec;
	char cdate[7];
	char buf[70];

	//Oznaczenie gotowego zestawu danych
	//bit 0 - zestaw 0
	//bit 1 - zestaw 1
	uint8_t ready;

	//Tablice zestawów
	uint32_t np0;
	uint32_t np1;
	uint32_t np2;

	//Znacznik czasu
	int8_t t;
} s_nmea;

typedef struct {
	//Tryb pocz¹tkowy wariometru
	//0 - wy³¹czony w TX i RX
	//1 - w³¹czony w TX / wy³¹czony w RX
	//2 - w³¹czony w TX i RX
	uint8_t vmodeinit;
} s_config;

typedef struct {
	uint8_t numsat;

	//Wartoœæ alarmu akrobatycznego
	int16_t alarm;

	//Wartoœæ warioalarmu
	int16_t valarm;

	//uint8_t gain;
	//uint8_t rssi;

	//W³¹cznik trybu wariometru
	uint8_t variomode;

	//Statusy
	uint8_t status;
	uint8_t prev_status;
} s_rxd;

extern volatile uint16_t t_rx;
extern volatile uint16_t t_nmea;
extern volatile uint16_t t_rxpos;

extern s_rxd rxd;
extern s_nmea nmea;
extern s_config conf;

//void setBuzzer(int16_t val);
void rx_loop();
void rx_init();
void nmea_decode();

#endif
#endif /* _F3A_RX_H_ */
