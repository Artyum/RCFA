/*
 * sd.h
 *
 *  Created on: 8 maj 2015
 *      Author: awitczak
 */

#ifndef SD_SD_H_
#define SD_SD_H_

#include "../config.h"
#if RADIO_MODE==1

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <avr/pgmspace.h>

#include "ff.h"
#include "diskio.h"
#include "../UART/uart.h"
#include "../RCFA/rcfa.h"
#include "../GPS/gps.h"
#include "../SPI/spi.h"
#include "../CALC/calc.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

//#include <avr/eeprom.h>
#include "ff.h"
#include "diskio.h"
#include "../RCFA/rcfa.h"

//#define SD_DELAY			1
#define SD_ITER				100
#define SD_MAX_FNUM			999
#define SD_MAX_BUF			5

#define LOGSPEED_MIN		0.2
#define LOGSPEED_MAX		60
#define PATH_FREQ_MIN		0
#define PATH_FREQ_MAX		250
#define TZONE_MIN			-12
#define TZONE_MAX			14

//Ustawienia domyœlne
#define INI_DEF_HLIMIT		0
#define INI_DEF_HLOWLIMIT	0
#define INI_DEF_KALMAN		1
#define INI_DEF_BARO		1
#define INI_DEF_LOCKSPEED	15
#define INI_DEF_VOL			100
#define INI_DEF_LOWACC		0
#define INI_DEF_LOGLEVEL	1
#define INI_DEF_VARIOMODE	1
#define INI_DEF_HSPMIN		0.4
#define INI_DEF_HSPMAX		5
#define INI_DEF_CR			1.0
#define INI_DEF_CQ			1.0
#define INI_DEF_AR			3.5
#define INI_DEF_AQ			1.0
#define INI_DEF_VR			4.0
#define INI_DEF_VQ			1.0
#define INI_DEF_RADIO		1
#define INI_DEF_TEST		0
#define INI_DEF_TXPOS		0
#define INI_DEF_REFSAT		9
#define INI_DEF_REFALT		0
#define INI_DEF_MODE		1
#define INI_DEF_TZONE		0
#define INI_DEF_GPSHZ		5
#define INI_DEF_LOGSPEED	0.5
#define INI_DEF_FLOOR		0

//Przedzia³y
#define INI_VOL_MIN			0
#define INI_VOL_MAX			10
#define INI_SAT_MAX			12
#define INI_HSP_MIN			0
#define INI_HSP_MAX			50
#define INI_DEBUG_MIN		0
#define INI_DEBUG_MAX		50
#define INI_HLIMIT_MAX		10000
#define GPS_MIN_ALT			-500
#define GPS_MAX_ALT			6000
#define GPS_REFSAT_MIN		GPS_MIN_NUMSAT
#define GPS_REFSAT_MAX		100

//S³ownikowe
#define GE_NOT_VISIBLE		0x01
#define GE_ALT_ABSOLUTE		0x02
#define GE_ALT_REL2GND		0x04
#define GE_STYLE_PL			0x08
#define GE_STYLE_LW			0x10
#define GE_STYLE_AW			0x20
#define GE_EXTRUDE			0x40

//Maksymalna odleg³oœæ miêdzy kolejnymi punktami z gps [m] - zabezpieczenie przed zak³óceniami gps
//#define LOGGER_MAX_DIST			100

//Nazwy plików
#define FNAME_KML		"rcfa"
#define FNAME_INI		"rcfa.ini"
#define FNAME_LOG		"rcfa.log"

//Opcje INI
#define INI_D			"dist"
#define INI_K			"angle"
#define INI_SB			"awidth"
#define INI_SC			"lwidth"
#define INI_MARGIN		"margin"
#define INI_HLIMIT		"hlimit"
#define INI_HLOWLIMIT	"hlowlimit"
#define INI_RADIO		"radio"
#define INI_LOGMODE		"logger"
#define INI_LOGSPEED	"logfreq"
#define INI_PATHFREQ	"pathfreq"
#define INI_FLOOR		"floor"
#define INI_GPSHZ		"gpshz"
#define INI_TZONE		"tzone"
#define INI_FILTER		"filter"
#define INI_CR			"cr"
#define INI_CQ			"cq"
#define INI_AR			"ar"
#define INI_AQ			"aq"
#define INI_BARO		"altimeter"
#define INI_LSPEED		"lockspeed"
#define INI_DEBUG		"log"
#define INI_Z_LAT		"p1lat"
#define INI_Z_LON		"p1lon"
#define INI_P_LAT		"p2lat"
#define INI_P_LON		"p2lon"
#define INI_VOL			"volume"
#define INI_LOWACC		"lowacc"
#define INI_VARIOMODE	"variomode"
#define INI_HSPMIN		"hspmin"
#define INI_HSPMAX		"hspmax"
#define INI_VR			"vr"
#define INI_VQ			"vq"
#define INI_TEST		"test"
#define INI_TXPOS		"txpos"
#define INI_REFSAT		"refsat"
#define INI_REFALT		"refalt"

//Specjalne tryby debugowania
#define DEBUG_GPS		11
#define DEBUG_ALT		12
#define DEBUG_VARIO		13

//Co ile zapisów nast¹pi sync przy zapisie wspó³rzêdnych w kml
#define SYNC_CNT		10

//Formatownie czasu
#define TIME_UTC		0
#define TIME_TZONE		1

typedef struct {
	s_wsp3d w3d;
	char cdate[7];
	char ctime[7];
} s_logbuf;

typedef struct {
	//Nazwa pliku
	char fname[25];

	//Status loggera
	// 0 - Wy³¹czony | 1 - Aktywny
	uint8_t enabled;

	//Bufor logera
	s_logbuf buf[SD_MAX_BUF];

	//Indeks w buforze
	uint8_t i;

	//Zliczanie czasu, gdy samolot znajduje siê w centrum - GreenZone Time
	uint32_t gztime;

	//Czas rozpoczêcia logowania (w sek i w char)
	uint32_t tstart;

	//Aktualny czas z GSP UTC hhmmss
	char ctime[7];

	//Maksymalna zarejestrowana prêdkoœæ w wêz³¹ch
	double maxspeed;

	//Maksymalna zarejestrowana wysokoœæ
	double maxalt;

	//Maksymalny dopouszczalny czas pomiêdzy kolejnymi odczytami GPS (zalezny od logger.gpshz)
	//Ustawiane w logger_read_ini
	uint16_t gpsBreak_time;

	//Logowanie animowanej œcie¿ki przez plik tymczasowy
	// 0 - Wy³¹czone | 1-250 - W³¹czone
	//uint8_t pathfreq;
} s_logger;

//Zmienne
//extern FATFS FatFs;
//extern FIL Fil;
//extern FIL Flog;
extern s_logger logger;

extern volatile uint16_t t_logger_buf;	//Timer zapisu danych do bufora loggera GPS
extern volatile uint16_t t_sd_proc;		//Procedura sprawdzania dostêpnoœci karty co 10ms

extern const char txt44[];
extern const char txt45[];
extern const char txt46[];
extern const char txt47[];
extern const char txt48[];
extern const char txt49[];
extern const char txt50[];
extern const char txt51[];
extern const char txt52[];
extern const char txt53[];
extern const char txt54[];
extern const char txt55[];
extern const char txt56[];
extern const char txt57[];
extern const char txt59[];
extern const char txt60[];
extern const char txt61[];
extern const char txt62[];
extern const char txt63[];
extern const char txt67[];
extern const char txt68[];
extern const char txt70[];
extern const char txt71[];
extern const char txt72[];
extern const char txt73[];
extern const char txt74[];
extern const char txt75[];
extern const char txt76[];
extern const char txt77[];
extern const char txt78[];
extern const char txt79[];
extern const char txt80[];
extern const char txt81[];
extern const char txt82[];
//extern const char txt83[];
extern const char txt84[];
extern const char txt85[];
extern const char txt86[];
extern const char txt87[];

uint8_t sd_log_init();
void logger_read_ini();
void logger_open();
void logger_close();
void logger_write_buf();

void sd_log(uint8_t lv, char *str);
void sd_log_val(uint8_t lv, char *s, double vd, uint16_t vi);
void sd_logp(uint8_t lv, void *adr);

void memtest(uint8_t print);

FRESULT sd_umount();

#endif
#endif /* SD_SD_H_ */
