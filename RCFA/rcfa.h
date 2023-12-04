/*
 * f3a.h
 *
 *  Created on: 19 mar 2015
 *      Author: Dexter
 */

#ifndef _F3A_H_
#define _F3A_H_

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "../config.h"

//Wartoœci domyœlne strefy
#define F3A_DEF_D			150.0
#define F3A_DEF_K			60.0
#define F3A_DEF_SB			270.0
#define F3A_DEF_SC			30.0
#define F3A_DEF_MARGIN		0

#define F3A_D_MIN			10.0
#define F3A_D_MAX			2000.0
#define F3A_K_MIN			1.0
#define F3A_K_MAX			85.0
#define F3A_SB_MIN			10.0
#define F3A_SB_MAX			5000.0
#define F3A_SC_MIN			1.0

//Min i Max odleg³oœci miêdzy punktami Z i P w metrach
#define F3A_MIN_ZP			0.1
#define F3A_MAX_ZP			500.0

//Przesuniêcie przedzia³u alarmu z -10k;10k => 10k;30k - by zapobiec przesy³aniu przez radio liczb ca³kowitych bliskich 0
#define ALARM_PUSH			20000L
#define ALARM_MIN			-10000L
#define ALARM_MAX			10000L

//Maksymalna dopuszczalna odleg³oœæ miêdzy L i P1 do wyliczenia strefy
#define RCFA_MAX_DIST_LP1	200

//Maksymalna prêdkoœæ przy której dzia³a filtr dolnoprzepustowy na wysokoœæ i pozycjê [wêz³y]
#define STEADY_SPD			1.5

//Czêstotliwoœæ odczytu danych z GPS
#define GPSDATA_DELAY		50

//Czêstotliwoœæ obliczania pozycji w strefie i naliczania punktów
#define CHKPOS_DELAY		GPSDATA_DELAY

//Szybkoœæ migania diodami
#define SYSTEM_STOP_DELAY	200

//Znacznik odczytanych wspó³rzêdnych z SD (4 wartoœci)
#define AREA_SET_SD			4

//Czêstotliwoœæ sprawdzania vario
#define VARIO_DELAY			100

//Liczba próbek do uœredniania valarm (0,5 sek)
#define VARIO_MAX_AVG_CNT	(1000/VARIO_DELAY/2)

//Czas migniêcia diod¹
#define BLINK_TIME			300


//Wspó³rzêdne geograficzne
typedef struct {
	double lat;		//Szerokoœæ
	double lon; 	//D³ugoœæ
} s_wsp;


//Po³o¿enie gps + wysokoœæ
typedef struct {
	s_wsp w;		//Wspó³rzêdne
	double alt;		//Wysokoœæ
} s_wsp3d;


typedef struct {
	double dist;	//Odleg³oœæ od linii lotów
	double angle;	//K¹t strefy [st.] 1-179 st.
	double sb;		//Szerokoœæ Boxa [m]; strefa alarmu = +/-(sb/2) od lini lotu
	double sc;		//Szerokoœæ linni lotu [m]; = +/-(sc/2) od linni lotu
	uint8_t margin;	//Alarm graniczny A/B/W

	double h;		//Wysokoœæ strefy [m]
	double ss;		//Ca³kowita szerokoœæ strefy [m] od pkt A do B
	double radius;	//Radii of Curvature

	//Pomocnicze
	double sb2;	//=sb/2
	double sc2;	//=sc/2

	s_wsp P1;		//Zawodnik
	s_wsp P2;		//Kierunek na œrodek
	s_wsp C;		//Punkt œrodka strefy
	s_wsp A;		//Punkt A
	s_wsp B;		//Punkt B
	s_wsp3d L;		//Aktualne po³o¿enie samolotu
	s_wsp3d prevL;	//Poprzednie po³o¿enie (do filtrowania)

	//Wspó³rzêdne wierzcho³ków boxa wyznaczaj¹cego strefê linii lotu
	s_wsp SAn;		//A near
	s_wsp SAf;		//A far
	s_wsp SBn;		//B near
	s_wsp SBf;		//B far

	s_wsp AAn;		//A near
	s_wsp AAf;		//A far
	s_wsp ABn;		//B near
	s_wsp ABf;		//B far

	//Stopieñ alarmu dla pozycji w strefie lub wariometru
	//Przedzia³: <-10'000;10'000>
	int16_t alarm;

	//Wartoœæ wariometru
	//Przedzia³: <-10'000;10'000>
	int16_t	valarm;

	//Status: 0-Nieaktualne; 1-Aktualne
	// bit 0 - Punkt Z
	// bit 1 - Punkt P
	// bit 2 - Punkty S,A,B
	// bit 3 - Punkty ustalone - blokada przycisku
	// bit 4 - Wczytano poprzedni¹ strefê
	uint8_t status;

	//Punkt referencyjny dla NMEA
	s_wsp R;

	//Wysokoœæ referencyjna z GPS i barometru
	double refalt;		//Wysokoœæ z baro lub z gps
	double refalt_gps;	//Wysokoœæ z GPS

	//Znaczniki wstêpnego ustalenia wysokoœci
	uint8_t refalt_prep;

	//Licznik pomijanych odczytów GPS
	uint8_t gpspos_prep;

	double bearing;
} s_rcfa;


typedef struct {
	//Wspó³rzêdne z INI
	uint8_t iniwspcnt;	//Znacznik czy odczytano 4 wspó³rzêdne z ini. Jeœli =4 to znaczy ¿e odczytano 4 wspó³rzêdne.

	//W³¹cznik modu³u radiowego
	uint8_t radio;

	//W³¹cznik barometru (MPL3115A2)
	uint8_t baro;

	//Uwzglêdnienie wysokoœci strefy przy obliczaniu alarmu
	uint16_t hlimit;
	uint16_t hlowlimit;

	//Prêdkoœæ przy której nastêpuje automatyczna blokada strefy w kph
	uint8_t lockspeed;

	//Szczegó³owoœæ logowania (0-wy³)
	uint8_t loglevel;

	//Prze³¹cznik wariometru
	//0 - wy³¹czony, dane nie s¹ wysy³ane do odbiornika
	//1 - w³¹czony - odbiornik ustawiony w trybie akrobatycznym
	//2 - w³¹czony - odbiornik ustawiony w trybie wariometru
	uint8_t variomode;

	//Maksymalna prêdkoœæ pionowa, poni¿ej której uznaje siê lot poziomy
	double hspmin;

	//Maksymalna wykrywana prêdkoœæ pionowa
	double hspmax;

	//Konfiguracja filtru kalmana
	uint8_t filter;
	double cr;
	double cq;
	double ar;
	double aq;
	double vr;
	double vq;

	//Prêdkoœæ odczytu GPS w Hz: 1|5|10
	uint8_t gpshz;

	//Timezone
	int8_t tzone;

	//Co ile sekund nast¹pi logowanie wspó³rzêdnych
	double logspeed;

	//Tryb loggera
	// 0 - Wy³¹czony | 1 - Automatyczny | 2 - Manualny
	uint8_t loggermode;

	//Minimalna liczba satelit, przy której sygna³ uznawany jest za dobry. Poni¿ej tej liczby pojawia siê alarm ma³ej ma³ej dok³adnoœci GPS
	uint8_t lowacc;

	//Znacznik czy schodziæ z wysokoœci¹ poni¿ej 0
	uint8_t floor;

	//G³oœnoœæ startowa
	uint8_t vol;

	//Test urz¹dzenia - odczyt danych z pamiêci EEMEM i inne testy
	// 0 - wy³¹czone
	// 1 - Everest
	// 2 - Everest + Memtest
	uint8_t test;

	//W³¹cznik wysy³ania danych NMEA
	uint8_t txpos;

	//Minimalna liczba satelitów potrzebna do uzyskania odczytu wysokoœci referencyjnej
	uint8_t refsat;

	//Wysokoœæ referencyjna ustawiana rêcznie
	double refalt_gps;
} s_ini;

typedef struct {
    uint8_t radio:1;
    uint8_t gps:1;
    uint8_t mpl:1;
    uint8_t sdmount:1;
    uint8_t sdlog:1;
    uint8_t sdini:1;
    uint8_t bit6:1;
    uint8_t bit7:1;
} s_hw;

//Operacje bitowe
#define SET_P1			0x01
#define SET_P2			0x02
#define SET_CAB			0x04
#define SET_BLK			0x08
#define SET_PREV		0x10
#define SET_P1P2		(SET_P1 | SET_P2)
#define SET_RTB			(SET_P1P2 | SET_CAB)

//Status sprzêtowy
//#define HW_RADIO		0x01
//#define HW_GPS		0x02
//#define HW_SD_MOUNT	0x04
//#define HW_SD_INI		0x08
//#define HW_MPL		0x10
//#define HW_SD_LOG		0x20
//#define HW_ALL_OK		(HW_RADIO | HW_GPS | HW_SD_MOUNT | HW_SD_INI | HW_MPL)

double knots2kph(double k);
double kph2knots(double k);

#if FUNC_MAP==1
double map(double v, double f1, double f2, double t1, double t2);
#endif
void system_stop(void);

extern s_rcfa rcfa;
extern s_ini ini;
extern uint8_t hw_status;
extern s_hw hw;

#endif /* _F3A_H_ */
