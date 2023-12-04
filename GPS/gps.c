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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <avr/pgmspace.h>
#include "gps.h"
#include "../config.h"
#include "../UART/uart.h"
#include "../SD/sd.h"
#include "../KALMAN/kalman.h"
#include "../MPL3115A2/mpl3115a2.h"
#include "../RCFA/rcfa_tx.h"
#include "../CALC/calc.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

#if RADIO_MODE==1

//Pauza pomiêdzy odczytami z GPS
volatile uint16_t t_getGPSdata;

//Sprawdzenie czasu pomiêdzy poprawnymi odczytami - GPS_MAX_BREAK
volatile uint16_t t_gpsBreak;

//Globalna zmienna z danymi odczyanymi z GPS
s_gpsData gpsData;

//Zmienna danych NMEA
s_nmea nmea;

uint8_t check_crc(char *s) {
	char crc[3];

	uint8_t len = strlen(s);
	if (len<5) return 0;

	//CRC calculation
	gps_crc(s,crc);

	//Check CRC (2 chars)
	if ((s[len-2]==crc[0]) && (s[len-1]==crc[1])) return 1;
	else return 0;
}

uint8_t chk_angles(s_wsp w) {
	if ((w.lat<-90.0) || (w.lat>90.0) || (w.lon<-180.0) || (w.lon>180.0)) return 0;
	else return 1;
}

//Change deg.min to float
double gps2float(int st, double min) {
	return (double)st + (min/60.0);
}

//Function to send PMTK command
void gpsCmd(uint8_t type, char *cmd) {
	char fcmd[60]; //Command array
	char scrc[3];

	//Start command
	if (type==PROTO_PMTK) { strcpy(fcmd,"$PMTK"); }
	else /*if (type==PROTO_UBX)*/ { strcpy(fcmd,"$PUBX,"); }

	//Add komendy
	strcat(fcmd, cmd);
	strcat(fcmd, "*");

	//Calc CRC
	gps_crc(fcmd, scrc);

	//Add CRC
	strcat(fcmd, scrc);

	//Finish command
	strcat(fcmd, "\r\n\0");

	//Send to GPS
	uart_puts(fcmd);

	//Wait for GPS response
	delay_ms(50);
}

void ubxCmd(uint8_t tab[], uint8_t cnt) {
	for (uint8_t i=0; i<cnt; i++) uart_putc(tab[i]);
	delay_ms(50);
}

uint8_t gpsFindModule() {
	uint8_t i=0;
	uint8_t spd[] = { UBRR_38400, UBRR_9600, UBRR_4800, UBRR_14400, UBRR_19200, UBRR_57600 };

	rmc_ready=0;

	//Próba odczytu RMC z ró¿nymi prêdkoœciami
	while ((rmc_ready==0) && (i<5)) {
		uart_disable();
		delay_ms(5);
		uart_init(spd[i++]);

		//Oczekiwanie na odebranie wiadomoœci z GPS
		t_gpsBreak=2200;
		while ((t_gpsBreak) && (rmc_ready==0));
	}

	if (rmc_ready) {
		rmc_ready = 0;
		return spd[i-1];
	}
	else {
		uart_disable();
		return 0;
	}
}

//Inicjalizacja GPS poprzez wys³anie odpowiednich komend
void gsp_init(void) {
	//Chipsety
	//Global Top:	MT3318, MT3329, MT3339
	//u-Blox:		5, 6, 7, M8 Series

	//Odszukanie modu³u GPS
	if (!gpsFindModule()) return;

	//UBX protocol command tabs

	//NAV5 | Dynamic Model Airborne<2g
	uint8_t tab_ubx_dm[] = { 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x0A };

	//RATE | 1Hz | UTC Time
	uint8_t tab_ubx_1hz[] = { 0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xE8, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x37 };

	//RATE | 5Hz | UTC Time
	uint8_t tab_ubx_5hz[] = { 0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x00, 0x00, 0xDD, 0x68 };

	//RATE | 10Hz | UTC Time
	uint8_t tab_ubx_10hz[] = { 0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x00, 0x00, 0x79, 0x10 };

	//PRT | Port Config 38400 | UART 1
	uint8_t tab_ubx_port[] = { 0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0x96, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x93, 0x90 };

	uint8_t tab_ubx_gga[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x05, 0x38 };		//GGA ON
	uint8_t tab_ubx_rmc[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x09, 0x54 };		//RMC ON
	uint8_t tab_ubx_gll[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x2B };		//GLL OFF
	uint8_t tab_ubx_gsa[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x32 };		//GSA OFF
	uint8_t tab_ubx_gsv[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x39 };		//GSV OFF
	uint8_t tab_ubx_vtg[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x47 };		//VTG OFF
	uint8_t tab_ubx_grs[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x4D };		//GRS off
	uint8_t tab_ubx_gst[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x54 };		//GST OFF
	uint8_t tab_ubx_zda[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5B };		//ZDA OFF
	uint8_t tab_ubx_gbs[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x62 };		//GBS OFF
	uint8_t tab_ubx_dtm[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x69 };		//DTM OFF
	uint8_t tab_ubx_gns[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x7E };		//GNS OFF

	//-------------------------------------------------------------

	//Pakiet testowy
	//gpsCmd("000");

	//Set NMEA port baud rate
	gpsCmd(PROTO_PMTK, "251,38400");

	//gpsCmd(PROTO_UBX, "41,1,0007,0003,38400,0");
	ubxCmd(tab_ubx_port, NUM_ELEMENTS(tab_ubx_port));

	//Odszukanie nowego uart baud dla GPS
	if (gpsFindModule()!=UBRR_38400) return;
	//if (!gpsFindModule()) return 0;

	//NMEA sentence output frequencies
    //Kolejnoœæ: GLL | RMC | VTG | GGA | GSA | GSV
	gpsCmd(PROTO_PMTK, "314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"); 	//RMC & GGA

	//UBX Set Dynamic Model
	ubxCmd(tab_ubx_dm, NUM_ELEMENTS(tab_ubx_dm));

	//Wy³¹czenie zbêdnych danych w u-Blox
	//gpsCmd(PROTO_UBX, "40,GLL,0,0,0,0,0,0");
	//gpsCmd(PROTO_UBX, "40,GSA,0,0,0,0,0,0");
	//gpsCmd(PROTO_UBX, "40,GSV,0,0,0,0,0,0");
	//gpsCmd(PROTO_UBX, "40,ZDA,0,0,0,0,0,0");
	//gpsCmd(PROTO_UBX, "40,VTG,0,0,0,0,0,0");
	ubxCmd(tab_ubx_gga, NUM_ELEMENTS(tab_ubx_gga));
	ubxCmd(tab_ubx_rmc, NUM_ELEMENTS(tab_ubx_rmc));
	ubxCmd(tab_ubx_gll, NUM_ELEMENTS(tab_ubx_gll));
	ubxCmd(tab_ubx_gsa, NUM_ELEMENTS(tab_ubx_gsa));
	ubxCmd(tab_ubx_gsv, NUM_ELEMENTS(tab_ubx_gsv));
	ubxCmd(tab_ubx_vtg, NUM_ELEMENTS(tab_ubx_vtg));
	ubxCmd(tab_ubx_grs, NUM_ELEMENTS(tab_ubx_grs));
	ubxCmd(tab_ubx_gst, NUM_ELEMENTS(tab_ubx_gst));
	ubxCmd(tab_ubx_zda, NUM_ELEMENTS(tab_ubx_zda));
	ubxCmd(tab_ubx_gbs, NUM_ELEMENTS(tab_ubx_gbs));
	ubxCmd(tab_ubx_dtm, NUM_ELEMENTS(tab_ubx_dtm));
	ubxCmd(tab_ubx_gns, NUM_ELEMENTS(tab_ubx_gns));

	//Set NMEA port update rate
	//1Hz
	if (ini.gpshz==1) {
		gpsCmd(PROTO_PMTK, "220,1000");	//crc:1F
		ubxCmd(tab_ubx_1hz, NUM_ELEMENTS(tab_ubx_1hz));
	}

	//5Hz
	else if (ini.gpshz==5) {
		gpsCmd(PROTO_PMTK, "220,200");	//crc:2C
		ubxCmd(tab_ubx_5hz, NUM_ELEMENTS(tab_ubx_5hz));
	}

	//10Hz
	else {
		gpsCmd(PROTO_PMTK, "220,100");	//crc:2F
		ubxCmd(tab_ubx_10hz, NUM_ELEMENTS(tab_ubx_10hz));
	}

	//Clear EPO data stored in the GPS chip
	//gpsCmd("127"); //crc: 36

	//Speed threshold - jesli prêdkoœæ jest ni¿sza "od" pozycja nie zmienia siê
	//Chipset MT3329
	gpsCmd(PROTO_PMTK, "397,0.0");		//0m/s		wy³¹czone
	//gpsCmd("397,0.2");				//0.2m/s	crc: 3F
	//gpsCmd("397,2.0");				//2m/s		crc: 3F

	//Chipset MT3339
	gpsCmd(PROTO_PMTK, "386,0");		//0m/s		wy³¹czone

	//RESET
	//gpsData.lat = 0;
	//gpsData.fix = 0;
	gpsData.numsat = 0;

	//Liczba pocz¹tkowych próbek do wy¿arzenia filtra
	filter_reset();

	//Inicjalizacja poprawna
	hw.gps = 1;
}

// *str - przeszukiwany string
// *P   - wskaŸnik na aktualny znak w stringu
//  d   - znak rozdzielaj¹cy
char *strParse(char *str, uint8_t *p, const char d) {
	while ((*str!='\0') && (*(str+*p)!=d)) (*p)++;
	(*p)++; //Ustawienie wskaŸnika na kolejny znak po 'd'
	return str+*p;
}

void decodeRMC(s_gpsData *gps) {
	char st[5], min[11];
	uint8_t p=0;
	char *str;

	//Kopia linii RMC
	//strcpy(buf,(char *)rmc_buf);

	//Pocz¹tek - $GPRMC

	//UTC Time hhmmss.sss
	str = strParse((char*)rmc_buf, &p, ',');
	strncpy(gps->ctime, str, 6); gps->ctime[6]='\0';

	//Status A valid | V not valid
	str = strParse((char*)rmc_buf, &p, ',');

	//Latitude ddmm.mmmm
	str = strParse((char*)rmc_buf, &p, ',');
	strncpy(st, str, 2); st[2]='\0';
	strncpy(min, str+2, 7); min[7]='\0';
	gps->lat = gps2float(strtol(st,NULL,10),strtod(min,NULL));

	//N|S
	str = strParse((char*)rmc_buf, &p, ',');
	if (*str=='S') gps->lat = -(gps->lat);

	//Longitude dddmm.mmmm
	str = strParse((char*)rmc_buf, &p, ',');
	strncpy(st, str, 3); st[3]='\0';
	strncpy(min, str+3, 7); min[7]='\0';
	gps->lon = gps2float(strtol(st,NULL,10),strtod(min,NULL));

	//W|E
	str = strParse((char*)rmc_buf, &p, ',');
	if (*str=='W') gps->lon = -(gps->lon);

	//Speed over ground [knots]
	str = strParse((char*)rmc_buf, &p, ',');
	gps->speed = strtod(str,NULL);

	//Course over ground [stopnie]
	str = strParse((char*)rmc_buf, &p, ',');
	//gps->heading = strtod(p,NULL);

	//Data ddmmyy
	str = strParse((char*)rmc_buf, &p, ',');
	strncpy(gps->cdate, str, 6); gps->cdate[6]='\0';
}

void decodeGGA(s_gpsData *gps) {
	uint8_t p=0;
	char *str;

	//Kopia linii GGA
	//strcpy(buf,(char *)gga_buf);

	//Pocz¹tek - $GPGGA
	str = strParse((char *)gga_buf, &p, ',');	//UTC Time
	str = strParse((char *)gga_buf, &p, ',');	//Latitude
	str = strParse((char *)gga_buf, &p, ',');	//N/S
	str = strParse((char *)gga_buf, &p, ',');	//Longitude
	str = strParse((char *)gga_buf, &p, ',');	//W/E

	//Fix quality
	str = strParse((char *)gga_buf, &p, ',');
	//gps->fix = strtol(p,NULL,10);

	//Number of satellites being tracked
	str = strParse((char *)gga_buf, &p, ',');
	gps->numsat = strtol(str,NULL,10);

	//HDOP
	str = strParse((char *)gga_buf, &p, ',');

	//Altitude, Meters, above mean sea level
	str = strParse((char *)gga_buf, &p, ',');
	gps->alt = strtod(str,NULL);

	//M
	str = strParse((char *)gga_buf, &p, ',');

	//Height of geoid (mean sea level) above WGS84 ellipsoid
	str = strParse((char *)gga_buf, &p, ',');
	gps->hog = strtod(str,NULL);
	//gps->alt = gps->alt-(gps->hog);
}

uint8_t getGPSdata() {
	//Jeœli GPS nie jest dostêpny to wyjœcie
	//if ((hw_status & HW_GPS)==0) return 0;

	//Zwraca 1 gdy po³o¿enie poprawnie odczytane
	uint8_t ret = 0;

	#if 0
	//Testy
	rmc_ready = gga_ready = 1;

	strcpy_P((char*)rmc_buf, PSTR("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\0"));
	strcpy_P((char*)gga_buf, PSTR("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\0"));

	//strcpy(rmc_buf, "$GPRMC,195210.000,A,5215.7267,N,02054.9798,E,0.11,91.17,130315,,,A*50");
	//strcpy(gga_buf, "$GPGGA,195210.000,5215.7267,N,02054.9798,E,1,5,1.34,107.8,M,39.0,M,,*5E");

	//strcpy(rmc_buf, "$GPRMC,195343.000,V,,,,,2.91,343.33,130315,,,N*4F");
	//strcpy(gga_buf, "$GPGGA,195343.000,,,,,0,2,,,M,,M,,*43");
	#endif

	//If new GPS data available
	if ((rmc_ready) && (gga_ready)) {

		//Disable uart until frame is decoded
		uart_disable_rx();

		//CRC Check
		if ((check_crc((char *)rmc_buf)) && (check_crc((char *)gga_buf))) {

			//Czas do nastêpnego odczytu GPS
			t_gpsBreak = logger.gpsBreak_time;

			//Get new data
			decodeRMC(&gpsData);
			decodeGGA(&gpsData);

			if ((gpsData.numsat>=GPS_MIN_NUMSAT) && ((fabs(gpsData.lat)>3.0) || (fabs(gpsData.lon)>3.0))) {
				if (rcfa.gpspos_prep>0) rcfa.gpspos_prep--;
				else {
					//Ustawienie aktualnej pozyscji samolotu
					rcfa.L.w.lat = gpsData.lat;
					rcfa.L.w.lon = gpsData.lon;
					ret = 1;
				}
			}
		}

		//Reenable uart
		uart_enable_rx();
	}

	//If GPS hardware is lost
	if (t_gpsBreak==0) {
		t_gpsBreak = logger.gpsBreak_time;
		//gpsData.fix = 0;
		gpsData.numsat = 0;
		gga_ready = 0;
		rmc_ready = 0;
		sd_logp(4, &txt87);
	}

	return ret;
}

//Returns 1 if lan and lon are valid
uint8_t position_valid() {
	static uint8_t v;

	if ((gpsData.numsat>=GPS_MIN_NUMSAT) && (klat.ready) && (klon.ready) && (kalt.ready) && (rcfa.refalt_prep==REFALT_READY)) {
		//GPS FIX reestablished
		if ((v & GPS_LOG_FIX)==0) { sd_logp(3, &txt77); v |= GPS_LOG_FIX; }

		//Lowacc
		if ((gpsData.numsat<=ini.lowacc) && ((v & GPS_LOG_LOWACC)==0)) { sd_logp(1, &txt80); v |= GPS_LOG_LOWACC; }

		return 1;
	}
	else {
		//GPS FIX lost
		if (v & GPS_LOG_FIX) { sd_logp(3, &txt78); v &= ~(GPS_LOG_FIX); rcfa.gpspos_prep = GPS_RECOVERY_CNT; }
		if ((gpsData.numsat>ini.lowacc) && (v & GPS_LOG_LOWACC)) { sd_logp(1, &txt81); v &= ~(GPS_LOG_LOWACC); }

		return 0;
	}
}

uint16_t getGpsTimeSec() {
	//gpsData.time | hhmmss
	uint8_t h = (gpsData.ctime[0]-48)*10 + (gpsData.ctime[1]-48);
	uint8_t m = (gpsData.ctime[2]-48)*10 + (gpsData.ctime[3]-48);
	uint8_t s = (gpsData.ctime[4]-48)*10 + (gpsData.ctime[5]-48);
	uint16_t ret = (uint16_t)h*3600UL + (uint16_t)m*60UL + (uint16_t)s;

	return ret;
}

void nmea_encode() {
	#if 0
	//TEST
	rcfa.X.lat = 31.450575;
	rcfa.X.lon = -115.734276;
	rcfa.refalt = 1100;
	rcfa.refalt_gps = 1050;
	rcfa.L.w.lat = 31.443900;
	rcfa.L.w.lon = -115.725000;
	rcfa.L.alt = 1500;
	gpsData.speed = 111;
	#endif

	//1. t 1 bit

	//2. Lat 19 bit
	uint32_t lat = ((int32_t)((double)(rcfa.L.w.lat-rcfa.R.lat)*(double)ACC6)+PUSH_19BIT);
	if (lat>MAX_19BIT) lat=MAX_19BIT;

	//3. Lon 20 bit
	uint32_t lon = ((int32_t)((double)(rcfa.L.w.lon-rcfa.R.lon)*(double)ACC6)+PUSH_20BIT);
	if (lon>MAX_20BIT) lon=MAX_20BIT;

	//4. Alt 16 bit
	double altd = rcfa.L.alt-rcfa.refalt+rcfa.refalt_gps;
	if ((ini.floor>=2) || ((ini.floor==1) && (altd<rcfa.refalt_gps))) altd=rcfa.refalt_gps;
	uint32_t alti = ((double)(altd+(double)ALT_PUSH)/(double)ALT_DIV*(double)MAX_16BIT);
	if (alti>MAX_16BIT) alti=MAX_16BIT;

	//5. Speed 8 bit
	uint16_t spd = (gpsData.speed/(double)SPD_DIV*(double)MAX_8BIT);
	if (spd>MAX_8BIT) spd=MAX_8BIT;

	//Frames nnt(21) => 3+21bit
	//Timestamp
	nmea.tab[0] = nmea.tab[1] = nmea.tab[2] = (uint32_t)nmea.t << 21UL;

	nmea.tab[0] |= lat << 2UL;
	nmea.tab[0] |= (lon & 0xC0000)>>18UL;
	nmea.tab[1] |= (lon & 0x3FFFF)<<3UL;
	nmea.tab[1] |= (alti & 0xE000)>>13UL;
	nmea.tab[2] |= (alti & 0x1FFF)<<8UL;
	nmea.tab[2] |= spd;

	#if 0
	uart_puts("Encode\r\n");
	uart_puts("alti="); uart_putul(alti,10); uart_nl();
	uart_puts("lat="); uart_putul(lat,10); uart_nl();
	uart_puts("lon="); uart_putul(lon,10); uart_nl();
	uart_puts("spd="); uart_putul(spd,10); uart_nl();

	for (uint8_t i=0; i<NMEA_TAB_CNT; i++) {
		uart_puts("tn[");
		uart_putl(i,10);
		uart_puts("] = ");
		uart_putl(nmea.tab[i],2);
		uart_nl();
	}
	#endif
}

#endif

//Wyliczenie NMEA CRC pomiêdzy $ i * - XOR znaków; Zwraca string[3]
//s    - zdanie nmea w³¹cznie z '$' i '*'
//scrc - obliczone crc, 2 znaki hex uppercase
//http://siliconsparrow.com/demos/nmeachecksum.php
void gps_crc(char *s, char *scrc) {
	int crc = 0;
	uint8_t i = 1;

	while (s[i]!='*') crc = crc^s[i++];

	//Formatowanie liczby w HEX
	itoa(crc, scrc, 16);

	//Dopisanie wiod¹cego '0'
	if (crc<16) {
		scrc[1] = scrc[0];
		scrc[0] = '0';
	}
	//Zamiana na du¿e znaki
	scrc[0] = toupper(scrc[0]);
	scrc[1] = toupper(scrc[1]);
	scrc[2] = '\0';
}
