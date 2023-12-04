/*
 * f3a.c
 *
 *  Created on: 5 mar 2015
 *      Author: awitczak
 */

#include "../config.h"
#if RADIO_MODE==0

#include <avr/io.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <avr/pgmspace.h>
#include "rcfa_rx.h"
#include "../UART/uart.h"
#include "../RADIO/radio_rx.h"
#include "../SPI/spi.h"
#include "../BTN/btn.h"
#include "../GPS/gps.h"
#include "../SOUND/sound.h"
#include "../CALC/calc.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

volatile uint16_t t_rx;		//Liczony czas pomiêdzy odczytami z radia
volatile uint16_t t_nmea;	//Czêstotliwoœæ generacji ramek NMEA
volatile uint16_t t_rxpos;	//Czas pomiêdzy odczytami pakietów do NMEA

//Zmienna z danymi odbieranymi przez radio
s_rxd rxd;
s_config conf;
s_nmea nmea;

void rx_init() {
	//Inicjalizacja
	rxd.numsat = 0;
	rxd.status = 0;
	//rxd.prev_status = 0; //Nie zerowaæ!

	nmea.sec = 0;
	nmea.ready = 0;
	nmea.t = 0;

	conf.vmodeinit = 1;
	rxd.variomode = 0;
}

uint16_t calc_delay(int16_t val) {
	double ret = ((double)val/(double)ALARM_MAX)*(double)(BUZZER_DELAY_MAX-BUZZER_DELAY_MIN)+(double)BUZZER_DELAY_MIN;
	//uart_puts("Delay="); uart_putd(ret,2); uart_nl();
	return ret;
}

uint16_t calc_vario_delay() {
	double ret = (double)VARIO_TIME_MAX-(double)abs(rxd.valarm)/10000.0*(double)(VARIO_TIME_MAX-VARIO_TIME_MIN);
	//uart_puts("valarm="); uart_putl(rxd.valarm,10);
	//uart_puts(" | vdelay="); uart_putul((uint16_t)ret,10);
	//uart_nl();
	return ret;
}

/*uint8_t calc_key() {
	uint8_t key = ((rxd.valarm/100.0+100.0)/200.0)*(VARIO_KEY_MAX-VARIO_KEY_MIN)+VARIO_KEY_MIN;
	return key;
}*/

//Najni¿szy dŸwiêk FAH4
void set_sound() {
	//Do testów
	//t_rx = RX_MAX_WAIT;
	//snd.type |= RX_TYPE_P2;

	//Ok 15ms przed koñcem dŸwiêku - czas na generacje tablicy nowej czêstotliwoœci dŸwiêku
	if (t_buzzer>15) return;

	//Odczytano konfiguracjê
	if (snd.type & SND_TYPE_CONF) {
		if (snd.lp>12) snd.lp=0;
		if (snd.lp==0)				{ play_pause(INIT_PAUSE); uart_putsP(PSTR("CONF"),1); }
		else if (snd.lp==1)			play_freq(FD5, N8, snd.vol);
		else if (snd.lp==2)			play_pause(N8);
		else if (snd.lp==3)			play_freq(FF5, N16, snd.vol);
		else if (snd.lp==4)			play_pause(N16);
		else if (snd.lp==5)			play_freq(FG5, N16, snd.vol);
		else if (snd.lp==6)			play_pause(N16);
		else if (snd.lp==7)			play_freq(FA5, N16, snd.vol);
		else if (snd.lp==8)			play_freq(FG5, N16, snd.vol);
		else if (snd.lp==9)			play_freq(FF5, N16, snd.vol);
		else if (snd.lp==10)		play_freq(FG5, N16, snd.vol);
		else if (snd.lp==11)		play_freq(FF5, N16, snd.vol);
		else						{ play_pause(END_PAUSE); snd.type &= ~(SND_TYPE_CONF | SND_TYPE_NOSIG | SND_TYPE_NOFIX); snd.lp=0; return; }
		snd.lp++;
		return;
	}

	//Odebrano punkt Z (P1)
	else if (snd.type & SND_TYPE_P1) {
		if (snd.lp>8) snd.lp=0;
		if (snd.lp==0)				{ play_pause(INIT_PAUSE); uart_putsP(PSTR("P1"),1); }
		else if (snd.lp==1)			play_freq(FC5, N8, snd.vol);
		else if (snd.lp==2)			play_pause(N16);
		else if (snd.lp==3)			play_freq(FD5, N16, snd.vol);
		else if (snd.lp==4)			play_freq(FC5, N16, snd.vol);
		else if (snd.lp==5)			play_freq(FD5, N16, snd.vol);
		else if (snd.lp==6)			play_pause(N16);
		else if (snd.lp==7)			play_freq(FF5, N8K, snd.vol);
		else						{ play_pause(END_PAUSE); snd.type &= ~(SND_TYPE_P1 | SND_TYPE_NOSIG | SND_TYPE_NOFIX); snd.lp=0; return; }
		snd.lp++;
		return;
	}

	//Odebrano punkt P (P2)
	else if (snd.type & SND_TYPE_P2) {
		if (snd.lp>8) snd.lp=0;
		if (snd.lp==0)				{ play_pause(INIT_PAUSE); uart_putsP(PSTR("P2"),1); }
		else if (snd.lp==1)			play_freq(FC5, N8, snd.vol);
		else if (snd.lp==2)			play_pause(N16);
		else if (snd.lp==3)			play_freq(FD5, N16, snd.vol);
		else if (snd.lp==4)			play_freq(FC5, N16, snd.vol);
		else if (snd.lp==5)			play_freq(FD5, N16, snd.vol);
		else if (snd.lp==6)			play_freq(FG5, N16, snd.vol);
		else if (snd.lp==7)			play_freq(FF5, N8K, snd.vol);
		else						{ play_pause(END_PAUSE); snd.type &= ~(SND_TYPE_P2 | SND_TYPE_NOSIG | SND_TYPE_NOFIX); snd.lp=0; return; }
		snd.lp++;
		return;
	}

	//Znacznik gotowoœci wysokoœci referencyjnej
	else if (snd.type & SND_TYPE_REFPOS) {
		if (snd.lp>10) snd.lp=0;
		if (snd.lp==0)				play_pause(INIT_PAUSE);
		else if (snd.lp==1)			play_freq(FF5, N16, snd.vol);
		else if (snd.lp==2)			play_pause(N16);
		else if (snd.lp==3)			play_freq(FF5, N16, snd.vol);
		else if (snd.lp==4)			play_pause(N16);
		else if (snd.lp==5)			play_freq(FF5, N16, snd.vol);
		else if (snd.lp==6)			play_pause(N16);
		else if (snd.lp==7)			play_freq(FAH5, N16K, snd.vol);
		else if (snd.lp==8)			play_pause(N8);
		else if (snd.lp==9)			play_freq(FD6, N16K, snd.vol);
		else						{ play_pause(END_PAUSE); snd.type &= ~(SND_TYPE_REFPOS | SND_TYPE_NOSIG | SND_TYPE_NOFIX); snd.lp=0; return; }
		snd.lp++;
		return;
	}

	//Odczytano znacznik gotowoœci strefy
	else if (snd.type & SND_TYPE_RTB) {
		if (snd.lp>7) snd.lp=0;
		if (snd.lp==0)				play_pause(INIT_PAUSE);
		else if (snd.lp==1)			play_freq(FD5, N16, snd.vol);
		else if (snd.lp==2)			play_freq(FF5, N16, snd.vol);
		else if (snd.lp==3)			play_freq(FD5, N8, snd.vol);
		else if (snd.lp==4)			play_freq(FF5, N8, snd.vol);
		else if (snd.lp==5)			play_freq(FG5, N4, snd.vol);
		else if (snd.lp==6)			play_freq(FF5, N4K, snd.vol);
		else						{ play_pause(END_PAUSE); snd.type &= ~(SND_TYPE_RTB | SND_TYPE_NOSIG | SND_TYPE_NOFIX); snd.lp=0; return; }
		snd.lp++;
		return;
	}

	//Odczytano zablokowanie strefy
	else if (snd.type & SND_TYPE_BLK) {
		if (snd.lp>15) snd.lp=0;
		if (snd.lp==0)				play_pause(INIT_PAUSE);
		else if (snd.lp==1)			play_freq(FC5, N8, snd.vol);
		else if (snd.lp==2)			play_pause(N8);
		else if (snd.lp==3)			play_freq(FC5, N16, snd.vol);
		else if (snd.lp==4)			play_pause(N16);
		else if (snd.lp==5)			play_freq(FC5, N16, snd.vol);
		else if (snd.lp==6)			play_pause(N16);
		else if (snd.lp==7)			play_freq(FC5, N16, snd.vol);
		else if (snd.lp==8)			play_freq(FD5, N16, snd.vol);
		else if (snd.lp==9)			play_pause(N16);
		else if (snd.lp==10)		play_freq(FE5, N16, snd.vol);
		else if (snd.lp==11)		play_pause(N16);
		else if (snd.lp==12)		play_freq(FF5, N8, snd.vol);
		else if (snd.lp==13)		play_pause(N8);
		else if (snd.lp==14)		play_freq(FF5, N8, snd.vol);
		else						{ play_pause(END_PAUSE); snd.type &= ~(SND_TYPE_BLK | SND_TYPE_NOSIG | SND_TYPE_NOFIX); snd.lp=0; return; }
		snd.lp++;
		return;
	}

	//Brak sygna³u
	#if DISABLE_SND_NO_SIG==0
	if (snd.type & SND_TYPE_NOSIG) {
		//if (snd.lp>103) snd.lp=0;
		if (snd.lp<=100)			{ play_freq(FD5, N16, snd.vol); snd.lp=100; }
		else if (snd.lp==101)		play_freq(FC5, N16, snd.vol);
		else if (snd.lp==102)		play_freq(FAH4, N16K, snd.vol);
		else						{ play_pause(END_PAUSE*6); snd.type &= ~(SND_TYPE_NOSIG | SND_TYPE_NOFIX); snd.lp=0; return; }
		snd.lp++;
		return;
	}
	#endif

	//Obs³uga wariometru
	else if (rxd.variomode) {
		if (snd.type & SND_TYPE_VALARM) {
			static uint8_t dir;

			if (rxd.valarm==0 && snd.lp<150) {
				sound_off();
				LED1_OFF; LED2_OFF;
				//snd.type &= ~SND_TYPE_VALARM;
				return;
			}
			else {
				if (snd.lp<=150)		{ play_pause(INIT_PAUSE); snd.lp=150; }
				else if (snd.lp==151)	{ if (rxd.valarm>0) { dir=1; LED1_OFF; LED2_ON; } else { dir=0; LED1_ON; LED2_OFF; } play_freq(FG5, N16, snd.vol); }
				else if (snd.lp==152)	play_pause(N64);
				else if (snd.lp==153)	{ if (dir) play_freq(FA5, N16, snd.vol); else play_freq(FF5, N16, snd.vol); }
				else					{ play_pause(calc_vario_delay()); LED1_OFF; LED2_OFF; snd.lp=0; /*snd.type &= ~SND_TYPE_VALARM;*/ return; }

				snd.lp++;
				return;
			}
		}
		return;
	}

	//Brak Fixa
	else if ((snd.type & SND_TYPE_NOFIX) && (rxd.status & RX_BLK_SET)) {
		//if (snd.lp>5) snd.lp=0;
		if (snd.lp<=200)			{ play_freq(FC5, N16, snd.vol); snd.lp=200; }
		else if (snd.lp==201)		play_pause(N16);
		else if (snd.lp==202)		play_freq(FDH5, N16, snd.vol);
		else if (snd.lp==203)		play_pause(N16);
		else if (snd.lp==204)		play_freq(FCH5, N8K, snd.vol);
		else						{ snd.type &= ~(SND_TYPE_NOFIX | SND_TYPE_NOSIG); play_pause(END_PAUSE*6); snd.lp=0; return; }
		snd.lp++;
		return;
	}

	//Ob³uga alarmu
	else if (snd.type & SND_TYPE_ALARM) {
		//double delay;
		uint8_t mode;				//0 - Przed lini¹ "pik"; 1 - Za lini¹ "pik.pik"
		int16_t val = rxd.alarm;	//val w przedziale -10000; 10000
		static uint8_t outside;
		double fq1, fq2;

		#if 0
		/*v1.2d*/
		if (val<0) { val=-val; mode = 0; } else mode = 1;
		fq1 = FF5;
		fq2 = FDH5;
		#else
		/*v1.2e*/
		if (val<0) {
			val=-val;
			if (snd.lp==0) {
				mode = 0;
				fq1 = FH5;
			}
		}
		else {
			if (snd.lp==0) {
				mode = 1;
				fq1 = FF5;
				fq2 = FDH5;
			}
		}
		#endif

		//W centrum linni lotu
		if (val==0) {
			if (snd.lp==0) { play_freq(FGH5, N16, snd.vol*0.3); snd.lp++; }
			else { play_pause(N8K); snd.lp=0; /*snd.type &= ~SND_TYPE_ALARM;*/ }
			LED1_ON; LED2_ON;
		}

		//Poza stref¹ pik.pik.piK____pik.pik.pik
		else if ((val==ALARM_MAX) || (outside)) {
			if (val==ALARM_MAX) outside=1;

			if (snd.lp==0)			{ LED1_ON;	LED2_OFF;	play_freq(FC5, N16, snd.vol); snd.lp++; }
			else if (snd.lp==1)		{ LED1_OFF;	LED2_OFF;	play_pause(N16); snd.lp++; }
			else if (snd.lp==2)		{ LED1_OFF;	LED2_ON;	play_freq(FC5, N16, snd.vol); snd.lp++; }
			else if (snd.lp==3)		{ LED1_OFF;	LED2_OFF;	play_pause(N16); snd.lp++; }
			else if (snd.lp==4)		{ LED1_ON;	LED2_OFF;	play_freq(FC5, N16, snd.vol); snd.lp++; }
			else					{ LED1_OFF;	LED2_OFF;	play_pause(1300); snd.lp=0; outside=0; /*snd.type &= ~SND_TYPE_ALARM;*/ }
		}

		else {
			if (snd.lp==0)					{ if (mode) LED2_ON; else LED1_ON; play_freq(fq1, N32, snd.vol); snd.lp=1; }					//1 pik
			else if (snd.lp==1 && mode==0)	{ LED1_OFF; LED2_OFF; play_pause(calc_delay(val)); snd.lp=0; /*snd.type &= ~SND_TYPE_ALARM;*/ }	//przerwa i koniec
			else if (snd.lp==1 && mode==1)	{ LED1_OFF; LED2_OFF; play_pause(N32); snd.lp=2; }												//przerwa
			else if (snd.lp==2)				{ play_freq(fq2, N32, snd.vol); snd.lp=3; }														//2 pik
			else							{ play_pause(calc_delay(val)); snd.lp=0; /*snd.type &= ~SND_TYPE_ALARM;*/ }						//przerwa i koniec
		}
	}
}

void nmea_decode() {
	int32_t tmp;

	//Lat
	tmp = ((uint32_t)(nmea.np0 & 0x1FFFFC)>>2UL);
	nmea.w.lat = nmea.ref.lat + (double)((int32_t)tmp-(int32_t)PUSH_19BIT)/(double)ACC6;

	//Lon
	tmp = ((uint32_t)(nmea.np0 & 0x3)<<18UL) | ((uint32_t)(nmea.np1 & 0x1FFFF8)>>3UL);
	nmea.w.lon = nmea.ref.lon + (double)((int32_t)tmp-(int32_t)PUSH_20BIT)/(double)ACC6;

	//Alt
	tmp = ((uint32_t)(nmea.np1 & 0x7)<<13UL) | ((uint32_t)(nmea.np2 & 0x1FFF00)>>8UL);
	nmea.alt = (double)tmp/(double)MAX_16BIT*(double)ALT_DIV-(double)ALT_PUSH;

	//Speed
	tmp = (uint32_t)(nmea.np2 & 0xFF);
	nmea.spd = (double)tmp/(double)MAX_8BIT*(double)SPD_DIV;

	nmea.ready = 1;
}

//Zwraca hhmmss
void nmea_sec2time() {
	char str[3];
	uint16_t h,m,s;

	//Godzina
	h = nmea.sec / 3600;
	if (h==0) strcat(nmea.buf,"00");
	else { if (h<10) strcat(nmea.buf,"0"); utoa(h,str,10); strcat(nmea.buf,str); }

	//Minuta
	m = (nmea.sec / 60) % 60;
	if (m==0) strcat(nmea.buf,"00");
	else { if (m<10) strcat(nmea.buf,"0"); utoa(m,str,10); strcat(nmea.buf,str); }

	//Sekunda
	s = nmea.sec % 60;
	if (s==0) strcat(nmea.buf,"00");
	else { if (s<10) strcat(nmea.buf,"0"); utoa(s,str,10); strcat(nmea.buf,str); }
}

//Zwraca type: DEG2LAT->ddmm.mmmm; DEG2LON->dddmm.mmmm
void nmea_deg2coord(double deg, uint8_t type) {
	char tmp[8];
	unsigned int d;

	if (deg<0.0) deg=-deg;
	d = deg;
	deg = (deg-(double)d)*60.0;

	utoa(d, tmp, 10);
	if ((type==DEG2LON) && (d<100)) strcat(nmea.buf,"0");
	if (d<10) strcat(nmea.buf, "0");
	strcat(nmea.buf, tmp);

	dtostrf(deg,1,4,tmp);
	if (deg<10.0) strcat(nmea.buf, "0");
	strcat(nmea.buf, tmp);
}

void nmea_output() {
	static s_wsp wprev;
	static double brng;
	char tmp[11];

	//GGA
	strcpy_P(nmea.buf, PSTR("$GPGGA,"));
	nmea_sec2time();
	strcat_P(nmea.buf, PSTR(","));
	nmea_deg2coord(nmea.w.lat, DEG2LAT);
	if (nmea.w.lat>=0) strcat(nmea.buf, ",N,"); else strcat(nmea.buf, ",S,");
	nmea_deg2coord(nmea.w.lon, DEG2LON);
	if (nmea.w.lon>=0) strcat(nmea.buf, ",E"); else strcat(nmea.buf, ",W");
	strcat_P(nmea.buf, PSTR(",1,"));
	if (rxd.numsat<10) strcat(nmea.buf, "0");
	utoa(rxd.numsat, tmp, 10);
	strcat(nmea.buf, tmp);
	strcat_P(nmea.buf, PSTR(",1.0,"));
	dtostrf(nmea.alt,1,1,tmp);
	strcat(nmea.buf, tmp);
	strcat_P(nmea.buf, PSTR(",M,0,M,,*"));
	gpsCRC(nmea.buf, tmp);
	strcat(nmea.buf, tmp);
	uart_puts(nmea.buf);
	uart_nl();

	//RMC
	strcpy_P(nmea.buf, PSTR("$GPRMC,"));
	nmea_sec2time();
	strcat_P(nmea.buf, PSTR(",A,"));
	nmea_deg2coord(nmea.w.lat, DEG2LAT);
	if (nmea.w.lat>=0) strcat(nmea.buf, ",N,"); else strcat(nmea.buf, ",S,");
	nmea_deg2coord(nmea.w.lon, DEG2LON);
	if (nmea.w.lon>=0) strcat(nmea.buf, ",E"); else strcat(nmea.buf, ",W");
	strcat_P(nmea.buf, PSTR(","));

	//Speed
	dtostrf(nmea.spd,1,1,tmp);
	strcat(nmea.buf, tmp);
	strcat_P(nmea.buf, PSTR(","));

	//Bearing
	if ((wprev.lat!=nmea.w.lat) || (wprev.lon!=nmea.w.lon)) brng=calc_bearing(wprev, nmea.w);
	dtostrf(brng,1,1,tmp);
	strcat(nmea.buf, tmp);
	wprev.lat = nmea.w.lat;
	wprev.lon = nmea.w.lon;

	strcat_P(nmea.buf, PSTR(","));
	strcat(nmea.buf, nmea.cdate);
	strcat_P(nmea.buf, PSTR(",,*"));
	gpsCRC(nmea.buf, tmp);
	strcat(nmea.buf, tmp);
	uart_puts(nmea.buf);
	uart_nl();
}

/*
1. brak sygna³u
	- oczekiwanie na sygnal i konfiguracje
2. odebrano konfiguracje
	- oczekiwanie na gps i ref alt
3. jest gps i refalt
	+ mozna ustalic strefe (P1,P2)
	+ mozna uzywac wario
	- oczekiwanie na refpos
4. jest refpos
	+ mozna odczytywac nmea real-time
*/

void set_led() {
	//Dioda 1
	if (t_led1==0) {
		uint8_t sig = 0;
		if (rxd.status & RX_CONFIG_SET) sig = 1;
		if ((sig==1) && (rxd.numsat>=GPS_MIN_NUMSAT) && (rxd.status & RX_REFALT_SET)) sig = 2;
		if ((sig==2) && (rxd.status & RX_RTB_SET)) sig = 3;

		//Brak sygna³u / brak odczytanej konfiguracji
		if ((t_rx==0) || (sig==0)) {
			LED2_OFF;
			if (led1_status==0) { t_led1=100; LED1_ON; led1_status=1; }
			else { t_led1=2000; LED1_OFF; led1_status=0; }
			return;
		}

		//Jeœli strefa zablokowana to obs³uga diody odbywa siê w set_sound
		if ((rxd.status & RX_BLK_SET) && (rxd.numsat>=GPS_MIN_NUMSAT)) return;

		//Wyjœcie jeœli jest aktywny tryb wariometru
		if (rxd.variomode) return;

		//Odebrano konfiguracje
		if (sig==1) {
			if (led1_status==0) { t_led1=100; LED1_ON; led1_status=1; }
			else { t_led1=700; LED1_OFF; led1_status=0; }
			//uart_puts("1");
		}

		//Odebrano refalt i jest GPS
		else if (sig==2) {
			if (led1_status==0)	{ t_led1=100; LED1_ON; led1_status=1; }
			else if (led1_status==1) { t_led1=200; LED1_OFF; led1_status=2; }
			else if (led1_status==2) { t_led1=100; LED1_ON; led1_status=3; }
			else { t_led1=900; LED1_OFF; led1_status=0; }
			//uart_puts("2");
		}

		//Strefa gotowa do zablokowania
		else if (sig==3) {
			LED1_ON;
			t_led1=90;
			led1_status=0;
			//uart_puts("3");
		}

		//Dioda 2 zsynchronizowana z diod¹ 1
		if (t_led2==0) {
			if ((rxd.status & RX_REFPOS_SET) && (rxd.numsat>=GPS_MIN_NUMSAT)) {
				if (led2_status==0) { t_led2=90; LED2_ON; led2_status=1; }
				else { t_led2=890; LED2_OFF; led2_status=0; }
			}
		}
	}
}

void rx_loop() {
	//Inicjalizacja zmiennych
	rx_init();

	//Inicjalizacja g³oœnika
	uart_putsP(PSTR("Sound init"),1);
	sound_init();

	#if 0
	//Test przejœcia przez strefê
	rxd.alarm = ALARM_MIN;
	int8_t dir=1;
	while (1) {
		t_rx = RX_MAX_WAIT;

		snd.type |= SND_TYPE_ALARM;
		rxd.numsat = GPS_MIN_NUMSAT;

		if (t_nmea==0) {
			rxd.alarm += dir;
			if (rxd.alarm>ALARM_MAX) { rxd.alarm=ALARM_MAX; dir=-dir; }
			if (rxd.alarm<ALARM_MIN) { rxd.alarm=ALARM_MIN; dir=-dir; }

			if (rxd.alarm==0) t_nmea=3000;
			else if ((rxd.alarm==ALARM_MIN) || (rxd.alarm==ALARM_MAX)) t_nmea=3000;
			else t_nmea=1;
		}

		set_sound();
		delay_ms(1);
	}
	#endif

	//Test wariometru
	#if 0
	rxd.variomode = 1;
	double r = 0;
	while (1) {
		rxd.valarm = 10000 * sin(r);
		r = r+0.001;
		if (r>2*M_PI) r=0;

		//uart_puts("va:"); uart_putl(rxd.valarm,10); uart_nl();

		snd.type = SND_TYPE_VALARM;
		set_sound();
		delay_ms(10);
	}
	#endif

	//Inicjalizacja odbiornika - system zawiesza siê jeœli radio niedostêpne
	uart_putsP(PSTR("Radio init"),1);
	radio_init_rx();

	//Przegl¹d dŸwiêków
	if (BTN_PRESSED(BTN2_PINPORT,BTN2_PIN)) {
		uart_putsP(PSTR("Sound TEST"),1);
		while (BTN_PRESSED(BTN2_PINPORT,BTN2_PIN)) delay_ms(10);
		snd.type |= (SND_TYPE_CONF | SND_TYPE_P1 | SND_TYPE_P2 | SND_TYPE_REFPOS | SND_TYPE_RTB | SND_TYPE_BLK);
	}

	while (1) {
		key_check(&btn1_status, &btn1_timer, BTN1_PINPORT, BTN1_PIN);
		key_check(&btn2_status, &btn2_timer, BTN2_PINPORT, BTN2_PIN);

		//Krótkie naciœniêcie przycisków - zmiana g³oœnoœci
		if (btn1_status==KEY_UP_SHORT) chg_volume(10);
		if (btn2_status==KEY_UP_SHORT) chg_volume(-10);

		//Jeœli tryb wariometru jest dostêpny
		if (conf.vmodeinit>0) {
			//D³ugie naciœniêcie przycisku A - Tryb Acro
			if (btn1_status==KEY_DN_LONG_S) {
				rxd.variomode = 0;
				snd.type |= SND_TYPE_P1;
				LED1_OFF; LED2_OFF;
			}
			//D³ugie naciœniêcie przycisku B - Tryb Vario
			if (btn2_status==KEY_DN_LONG_S) {
				rxd.variomode = 1;
				snd.type |= SND_TYPE_P2;
				LED1_OFF; LED2_OFF;
			}
		}


		//Odbiór danych
		radio_rx_data();


		//Obs³uga dŸwiêku
		set_sound();


		//Obs³uga diod
		set_led();


		//Zmiany rxd.status
		if (rxd.prev_status!=rxd.status) {
			uint8_t tmp = rxd.prev_status ^ rxd.status;
			if ((tmp==RX_REFALT_SET) && (rxd.status & RX_REFALT_SET))		{ uart_putsP(PSTR("REFALT"),1);	snd.type |= SND_TYPE_REFPOS; }
			else if ((tmp==RX_REFPOS_SET) && (rxd.status & RX_REFPOS_SET))	{ uart_putsP(PSTR("REFPOS"),1);	snd.type |= SND_TYPE_REFPOS; }
			else if ((tmp==RX_RTB_SET) && (rxd.status & RX_RTB_SET))		{ uart_putsP(PSTR("RTB"),1);	snd.type |= SND_TYPE_RTB;	 }
			else if ((tmp==RX_BLK_SET) && (rxd.status & RX_BLK_SET))		{ uart_putsP(PSTR("BLK"),1);	snd.type |= SND_TYPE_BLK;	 }
			//else uart_putsP(PSTR("!"));

			rxd.prev_status = rxd.status;
		}

		//Wys³anie ramki NMEA
		if ((t_nmea==0) && (rxd.status & RX_REFPOS_SET)) {
			t_nmea = NMEA_GEN_TIME;
			if (nmea.ready) {
				#if CHK_RX==0
				if (rxd.numsat>=GPS_MIN_NUMSAT) nmea_output();
				#endif
				nmea.ready = 0;
			}
			nmea.sec++;
		}

		delay_ms(1);
	}
}
#endif
