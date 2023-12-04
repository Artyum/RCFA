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

#ifndef _GPS_H_
#define _GPS_H_

#include <avr/io.h>
#include "../config.h"
#include "../RCFA/rcfa.h"

//6 miejsc po przecinku dla wspó³rzêdnych GPS
#define ACC6				1000000

//1 miejsce po przecinku dla wysokoœci i prêdkoœci
#define ACC1				10

//Przesuniêcie wspó³rzêdnych przed wys³aniem przez radio
#define LAT_PUSH			90
#define LON_PUSH			180
#define ALT_PUSH			500

#define PUSH_16BIT			32767UL
#define PUSH_17BIT			65535UL
#define PUSH_18BIT			131071UL
#define PUSH_19BIT			262143UL
#define PUSH_20BIT			524287UL
#define MAX_6BIT			63
#define MAX_7BIT			127
#define MAX_8BIT			255
#define MAX_9BIT			511
#define MAX_10BIT			1023UL
#define MAX_14BIT			16383UL
#define MAX_15BIT			32767UL
#define MAX_16BIT			65535UL
#define MAX_17BIT			131071UL
#define MAX_18BIT			262143UL
#define MAX_19BIT			524287UL
#define MAX_20BIT			1048575UL

#define ALT_DIV				6553.5
#define SPD_DIV				127.5

//Minimalna liczba satelitów uznawana za fixa
#define GPS_MIN_NUMSAT		4

//Liczba pomijanych odczytów GPS po utracie fixa
#define GPS_RECOVERY_CNT	10

//Liczba przesy³anych bajtów
#define NMEA_TAB_CNT		3
#define REFPOS_TAB_CNT		11

#if RADIO_MODE==1

//Pominiêcie pocz¹tkowych odczytów GPS przy uruchomieniu systemu
#define GPS_START			30

//Maksymalny dopouszczalny czas pomiêdzy kolejnymi odczytami GPS (zalezny od logger.gpshz)
#define GPS_MAX_BREAK		5000
#define GPS_BREAK			2500

//Maksymalny dopuszczalny przeskok pomiêdzy punktami GPS [m]
#define GPS_MAX_HOP_M		200

//Maksymalna dopuszczalna wysokoœæ odczytana z GPS [m]
//#define GPS_MAX_ALT		8500

//Rodzaj GPSa
#define PROTO_PMTK			1
#define PROTO_UBX			2

#define GPS_LOG_FIX			0x01
#define GPS_LOG_LOWACC		0x02

//Liczba próbek wysokoœci do wstêpnego ustalenia wysokoœci punktu Z (P1)
#define REFALT_CNT			30

//Oznaczenie gotowoœci odczytu wysokoœci
#define REFALT_READY		0
#define REFALTGPS_NOT_SET	999999

typedef struct {
	//Tabela danych NMEA do wys³ania przez radio
	uint32_t tab[NMEA_TAB_CNT];

	//Znacznik czasu dla ramki
	uint8_t t;

	//Znacznik gotowoœci danych
	uint8_t ready;
} s_nmea;

typedef struct {
	//s_wsp w;
	double lat;
	double lon;
	double speed;		//Prêdkoœæ w wêz³ach
	//double heading;

	double alt;			//Wysokoœæ z uwzglêdnieniem HOG
	double hog;			//Height of geoid

	//uint8_t fix;		//0-brak fixa; >0 fix
	uint8_t numsat;		//Liczba œledzonych satelitów
	uint8_t numsat_prev;
	uint8_t numsat_min;
	uint8_t numsat_max;

	char ctime[7];		//Czas UTC	hhmmss\0
	char cdate[7];		//Data		ddmmyy\0
} s_gpsData;

extern volatile uint16_t t_getGPSdata;
extern volatile uint16_t t_gpsBreak;
extern s_gpsData gpsData;
extern s_nmea nmea;

//Funkcja uzupe³nia i wysy³a komendê PMTK
//void gpsCmd(char *cmd);

//Sprawdzenie crc odczytanego zdania nmea; 1-crc ok; 0-b³¹d
//uint8_t checkCRC(char *s);

//Inicjalizacja GPS poprzez wys³anie odpowiednich komend
void gsp_init(void);

//Zamiana deg.min na float
double gps2float(int st, double min);

//Odczyt danych z rekordów NMEA
void decodeRMC(s_gpsData *gps);
void decodeGGA(s_gpsData *gps);

uint8_t getGPSdata();			//Odczytanie wspó³rzêdnych oraz samplowanie poœrednich z podanym skokiem rate/sek
uint16_t getGpsTimeSec();		//Zwraca aktualn¹ godzinê w sekundach
uint8_t chk_angles(s_wsp w);
uint8_t position_valid();
void nmea_encode();

#endif

void gps_crc(char *s, char *scrc);

#endif /* _GPS_H_ */
