/*
 * f3a_tx.c
 *
 *  Created on: 19 mar 2015
 *      Author: Dexter
 */

#include "../config.h"
#if RADIO_MODE==1
#include "rcfa_tx.h"

//Wspó³rzêdne ostatniej strefy
s_wsp EEMEM wspP1 = { 0.0, 0.0 };
s_wsp EEMEM wspP2 = { 0.0, 0.0 };

volatile uint16_t t_vario;

//Obs³uga Wariometru
s_vario vario;

//Licznik zapobiegaj¹cy przed czêstym w³¹czaniem i wy³¹czeniem logera w trybie automatycznym
uint8_t lmcnt;

//Licznik opóŸniaj¹cy obliczanie NMEA
uint8_t cnt_nmea_encode;

//Wys³anie zdarzeñ
// 0 - nic
// 1 - Wys³anie konfiguracji do odbiornika
// 2 - Znacznik ustalenia pozycji referencyjnej
// 3 - Wys³anie punktu referencyjnego
// 4 - Znacznik ustalenia punktu P1
// 5 - Znacznik ustalenia punktu P2
// 6 - Wys³anie znacznika strefy gotowej do zablokowania
// 7 - Wys³anie znacznika zablokowania strefy
uint8_t txtype;

//Naprzemienne wysy³anie danych / licznik
uint8_t txtype_cnt;

//Zabezpieczenie przed wielokrotnym obliczaniem CAB
uint8_t tag_sab;

void area_load_from_eeprom() {
	rcfa.P1.lat = (double)eeprom_read_float((float *)&wspP1.lat);
	rcfa.P1.lon = (double)eeprom_read_float((float *)&wspP1.lon);
	rcfa.P2.lat = (double)eeprom_read_float((float *)&wspP2.lat);
	rcfa.P2.lon = (double)eeprom_read_float((float *)&wspP2.lon);

	if ((isnan(rcfa.P1.lat)) || (isnan(rcfa.P1.lon)) || (isnan(rcfa.P2.lat)) || (isnan(rcfa.P2.lon))) {
		rcfa.P1.lat = 0;
		rcfa.P1.lon = 0;
		rcfa.P2.lat = 0;
		rcfa.P2.lon = 0;
	}
}

void area_save() {
	for (uint8_t i=0; i<3; i++) {
		eeprom_busy_wait(); eeprom_update_float((float *)&wspP1.lat, rcfa.P1.lat);
		eeprom_busy_wait(); eeprom_update_float((float *)&wspP1.lon, rcfa.P1.lon);
		eeprom_busy_wait(); eeprom_update_float((float *)&wspP2.lat, rcfa.P2.lat);
		eeprom_busy_wait(); eeprom_update_float((float *)&wspP2.lon, rcfa.P2.lon);
	}
}

void memory_reset() {
	for (uint8_t i=0; i<3; i++) {
		eeprom_busy_wait(); eeprom_update_float((float *)&wspP1.lat, 0.0);
		eeprom_busy_wait(); eeprom_update_float((float *)&wspP1.lon, 0.0);
		eeprom_busy_wait(); eeprom_update_float((float *)&wspP2.lat, 0.0);
		eeprom_busy_wait(); eeprom_update_float((float *)&wspP2.lon, 0.0);
	}
}

void rcfa_init(void) {
	//Wyliczane zmiennych
	rcfa.h = rcfa.dist*tan(d2r(rcfa.angle));	//Wysokoœæ strefy
	rcfa.ss = rcfa.h*2.0;						//Szerokoœæ strefy dAB
	rcfa.radius = 0;
	rcfa.sb2 = rcfa.sb/2.0;
	rcfa.sc2 = rcfa.sc/2.0;

	//Wysokoœæ pilota nieustalona
	rcfa.refalt_prep = REFALT_CNT;
	rcfa.refalt_gps = REFALTGPS_NOT_SET;

	if (ini.hlimit>0) {
		if (ini.hlimit==1) ini.hlimit=rcfa.h;	//Wysokoœæ automatyczna
		if (ini.hlowlimit>ini.hlimit) ini.hlowlimit=ini.hlimit;
	}

	//Odczyt strefy z EEPROM jeœli nie odczytano z SD
	if (ini.iniwspcnt!=AREA_SET_SD) {
		//Odczyt ostatniej strefy z EEPROM
		area_load_from_eeprom();
	}

	//Jeœli w EEPROM i w INI nie by³o strefy to oznaczenie punktów P1,P2,C,A,B jako nieaktualnych
	if ((rcfa.P1.lat==0.0) || (rcfa.P1.lon==0.0) || (rcfa.P2.lat==0.0) || (rcfa.P2.lon==0.0)) rcfa.status = 0;

	//Jeœli odczytano poprzedni¹ strefê to oznaczenie punktów jako aktualnych i gotowych do zablokowania
	else {
		if (ini.iniwspcnt==AREA_SET_SD) sd_logp(1, &txt49); else sd_logp(1, &txt48);
		sd_log_val(3, "P1lat", rcfa.P1.lat, 0); sd_log_val(3, "P1lon", rcfa.P1.lon, 0);
		sd_log_val(3, "P2lat", rcfa.P2.lat, 0); sd_log_val(3, "P2lon", rcfa.P2.lon, 0);
		rcfa.status = (SET_P1 | SET_P2 | SET_PREV);
	}

	//Jeœli filtrowanie wy³¹czone
	if (!ini.filter) {
		klat.ready = 1;
		klon.ready = 1;
		kalt.ready = 1;
	}
}

uint8_t calc_SAB() {
	s_wsp X, T, kat;
	double dZP, k, d1, d2;

	sd_logp(1, &txt84);
	sd_log_val(5, "L.lat", rcfa.L.w.lat, 0);	sd_log_val(5, "L.lon", rcfa.L.w.lon, 0);
	sd_log_val(3, "P1lat", rcfa.P1.lat, 0);		sd_log_val(3, "P1lon", rcfa.P1.lon, 0);
	sd_log_val(3, "P2lat", rcfa.P2.lat, 0);		sd_log_val(3, "P2lon", rcfa.P2.lon, 0);

	//Sprawdzenie k¹tów
	uint8_t chk=1;
	if ((rcfa.P1.lat==rcfa.P2.lat) && (rcfa.P1.lon==rcfa.P2.lon)) chk=0;
	if (!chk_angles(rcfa.P1)) chk=0;
	if (!chk_angles(rcfa.P2)) chk=0;
	if (chk==0) {
		sd_logp(1, &txt75);
		return 0;
	}

	//Sprawdzenie odleg³oœci miêdzy punktami Z i P
	dZP = calc_dist(rcfa.P1, rcfa.P2);
	if ((dZP<(double)F3A_MIN_ZP) || (dZP>(double)F3A_MAX_ZP)) {
		sd_logp(1, &txt71);
		return 0;
	}

	//Bearing Z->P
	rcfa.bearing = calc_bearing(rcfa.P1, rcfa.P2);

	//Obliczenie punktu S
	calc_target(&T, rcfa.P1, rcfa.bearing, rcfa.dist);		//Sposób 1
	kat.lat = rcfa.dist*(rcfa.P2.lat-rcfa. P1.lat)/dZP;		//Sposób 2
	kat.lon = rcfa.dist*(rcfa.P2.lon-rcfa. P1.lon)/dZP;
	rcfa.C.lat = rcfa.P1.lat+kat.lat;
	rcfa.C.lon = rcfa.P1.lon+kat.lon;
	//Sprawdzenie, które z wyliczeñ jest dok³adniejsze
	d1 = fabs(rcfa.dist-calc_dist(rcfa.P1, T));
	d2 = fabs(rcfa.dist-calc_dist(rcfa.P1, rcfa.C));
	if (d1<d2) { rcfa.C.lat = T.lat; rcfa.C.lon = T.lon; }

	//Aktualizacja zmiennych
	rcfa.dist = calc_dist(rcfa.P1, rcfa.C);
	rcfa.bearing = calc_bearing(rcfa.P1, rcfa.C);

	//Obliczenie punktu X
	k = 75.0;
	d2 = 100.0;
	X.lat = 0.0;
	X.lon = 0.0;
	do {
		d1 = rcfa.dist/cos(d2r(k));
		calc_target(&T, rcfa.P1, bearing_chg(rcfa.bearing,k), d1);
		d1 = fabs(calc_dist(rcfa.P1, T)-d1);
		if ((d1<d2) || ((X.lat==0.0)&&(X.lon==0.0))) {
			d2 = d1;
			X.lat = T.lat;
			X.lon = T.lon;
		}
		k = k-0.2;
	} while ((k>=50.0) && (d1>=0.03));

	//uart_puts("k="); uart_putd(k,2); uart_nl();
	uart_puts("X="); uart_putd(X.lat,6); uart_puts("|"); uart_putd(X.lon,6); uart_nl();

	//Odleg³oœæ SX i nowy k¹t
	d1 = calc_dist(rcfa.C,X);
	kat.lat = rcfa.h*(X.lat-rcfa.C.lat)/d1;
	kat.lon = rcfa.h*(X.lon-rcfa.C.lon)/d1;

	//Obliczenie puntów A i B
	rcfa.A.lat = rcfa.C.lat+kat.lat;		rcfa.A.lon = rcfa.C.lon+kat.lon;
	rcfa.B.lat = rcfa.C.lat-kat.lat;		rcfa.B.lon = rcfa.C.lon-kat.lon;

	//K¹t ZS
	kat.lat = rcfa.sc2*(rcfa.C.lat-rcfa.P1.lat)/rcfa.dist;
	kat.lon = rcfa.sc2*(rcfa.C.lon-rcfa.P1.lon)/rcfa.dist;
	rcfa.SAn.lat = rcfa.A.lat-kat.lat;	rcfa.SAn.lon = rcfa.A.lon-kat.lon;
	rcfa.SAf.lat = rcfa.A.lat+kat.lat;	rcfa.SAf.lon = rcfa.A.lon+kat.lon;
	rcfa.SBn.lat = rcfa.B.lat-kat.lat;	rcfa.SBn.lon = rcfa.B.lon-kat.lon;
	rcfa.SBf.lat = rcfa.B.lat+kat.lat;	rcfa.SBf.lon = rcfa.B.lon+kat.lon;

	//Obliczenie wspó³rzêdnych boxa strefy
	kat.lat = rcfa.sb2*(rcfa.C.lat-rcfa.P1.lat)/rcfa.dist;
	kat.lon = rcfa.sb2*(rcfa.C.lon-rcfa.P1.lon)/rcfa.dist;
	rcfa.AAn.lat = rcfa.A.lat-kat.lat;	rcfa.AAn.lon = rcfa.A.lon-kat.lon;
	rcfa.AAf.lat = rcfa.A.lat+kat.lat;	rcfa.AAf.lon = rcfa.A.lon+kat.lon;
	rcfa.ABn.lat = rcfa.B.lat-kat.lat;	rcfa.ABn.lon = rcfa.B.lon-kat.lon;
	rcfa.ABf.lat = rcfa.B.lat+kat.lat;	rcfa.ABf.lon = rcfa.B.lon+kat.lon;

	//Logowanie wyniku obliczeñ
	sd_log_val(2, "Clat", rcfa.C.lat, 0);	sd_log_val(2, "Clon", rcfa.C.lon, 0);
	sd_log_val(2, "Alat", rcfa.A.lat, 0);	sd_log_val(2, "Alon", rcfa.A.lon, 0);
	sd_log_val(2, "Blat", rcfa.B.lat, 0);	sd_log_val(2, "Blon", rcfa.B.lon, 0);

	return 1;
}

int16_t check_position() {
	//double time; btn1_timer=0;

	double dZL = calc_dist(rcfa.P1, rcfa.L.w);
	if (dZL<=0.1) return rcfa.alarm;	//Jeœli L blisko Z to zwrot poprzedniej wartoœci alarmu

	//K¹t miêdzy kierunkiem P1->S a P1->L
	double angle = d2r(fmod(fmod(calc_bearing(rcfa.P1,rcfa.L.w)-rcfa.bearing,360.0)+540.0,360.0)-180.0);

	//Odleg³oœæ od linni lotu
	//<0 - przed lini¹
	//>0 - za lini¹
	double Ldst = dZL*cos(angle)-rcfa.dist;
	double LdstAbs = fabs(Ldst);

	double Sdst = dZL*sin(angle);
	double ss2 = rcfa.ss/2.0;

	//Odleg³oœæ L od granicy A [m]
	double oLA = ss2-Sdst;

	//Odleg³oœæ L od granicy B [m]
	double oLB = ss2+Sdst;

	#if 0
	time = btn1_timer;
	uart_puts("Ldst: "); uart_putd(Ldst,6); uart_nl();
	uart_puts("oLA:  "); uart_putd(oLA,6); uart_nl();
	uart_puts("oLB:  "); uart_putd(oLB,6); uart_nl();
	uart_puts("time:  "); uart_putul(time,10); uart_nl();
	#endif

	//Alarmy
	uint8_t ofa = 0;	//Znacznik Out-Of-Area
	double alarm = 0.0;

	//Odleg³oœæ od linni lotu
	double tmp = (LdstAbs-rcfa.sc2)/(rcfa.sb2-rcfa.sc2);
	if (tmp>alarm) alarm=tmp;

	//Alarm graniczny
	if ((rcfa.margin) && ((oLA<0.0) || (oLB<0.0))) ofa = 1;

	//Wysokoœæ lotu
	if (ini.hlimit) {
		if ((rcfa.L.alt-rcfa.refalt) > ini.hlimit) ofa = 1;
	}

	//Jeœli lot poni¿ej dolnej granicy
	if (ini.hlowlimit) {
		if ((rcfa.L.alt-rcfa.refalt) < ini.hlowlimit) ofa = 1;
	}
	else {
		//Lot poni¿ej pozycji pilota
		if (rcfa.L.alt < rcfa.refalt) ofa = 1;
	}

	//Ustawienie alarmu
	if ((ofa) || (alarm>1.0)) alarm = 1.0;

	//Minus jeœli samolot jest przed lini¹; plus jeœli za lini¹
	if (Ldst<0.0) alarm = -alarm;

	//Skalowanie alarmu
	alarm = alarm*(double)ALARM_MAX;

	//Punktacja za przebywanie w centrum strefy
	if ((ofa==0) && (LdstAbs<=rcfa.sc2)) logger.gztime += 1;

	//Logger - tryb automatyczny
	if (ini.loggermode==1) {
		if ((logger.enabled==0) && (Ldst>=-rcfa.sb2)) {
			sd_logp(1, &txt62);
			logger_open();
			lmcnt = ini.gpshz;
		}

		else if ((logger.enabled==1) && (Ldst<-rcfa.sb2) && (lmcnt==0)) {
			sd_logp(1, &txt67);
			logger_close();
		}

		if (lmcnt) lmcnt--;
	}

	return (int16_t)alarm;
}

int16_t calc_valarm() {
	double tmp = 0;

	//Przesuniêcie i sumowanie elementów do œredniej
	for (uint8_t i=0; i<=VARIO_MAX_AVG_CNT-2; i++) {
		vario.t_sub[i] = vario.t_sub[i+1];
		tmp += vario.t_sub[i];
		//uart_putd(vario.t_sub[i],3);
	}

	//uart_puts("alt:"); uart_putd(rcfa.L.alt,3);

	//Nowa wysokoœæ na koniec tabeli
	vario.t_sub[VARIO_MAX_AVG_CNT-1] = rcfa.L.alt;
	tmp += vario.t_sub[VARIO_MAX_AVG_CNT-1];

	//uart_puts("|sum:"); uart_putd(tmp,3);

	//Œrednia ostatnich wysokoœci
	double avr = tmp/(double)VARIO_MAX_AVG_CNT;

	//uart_puts("|avr:"); uart_putd(avr,3);

	//Ró¿nica œrednich
	tmp = avr - vario.prev_avg;

	//uart_puts("|sub:"); uart_putd(tmp,3);

	//Zapamiêtanie nowej œredniej
	vario.prev_avg = avr;

	//Zamiana na m/s
	//tmp = tmp / (VARIO_DELAY*(VARIO_MAX_AVG_CNT-1)/1000.0);
	tmp = tmp * 1000 / (double)VARIO_DELAY;

	//uart_puts("|ms:"); uart_putd(tmp,3);

	//Filtrowanie
	if (ini.filter) tmp = kalman(tmp, &kvario.xk, &kvario.pk, ini.vr, ini.vq);

	//Odciêcia
	if (fabs(tmp)<=ini.hspmin) tmp = 0;
	else {
		if (tmp > 0) tmp -= ini.hspmin;
		if (tmp < 0) tmp += ini.hspmin;
		if (tmp > ini.hspmax) tmp = ini.hspmax;
		if (tmp < -ini.hspmax) tmp = -ini.hspmax;
	}

	//Debug po filtrowaniu
	if (ini.loglevel==DEBUG_VARIO) {
		sd_log_val(DEBUG_VARIO, "VAlt", rcfa.L.alt, 0);
		sd_log_val(DEBUG_VARIO, "VOut", tmp, 0);
	}

	//uart_puts("|fin:"); uart_putd(tmp,3);

	//Skalowanie
	tmp = tmp*((double)ALARM_MAX/ini.hspmax);

	//uart_puts("|scale:"); uart_putd(tmp,3);
	//uart_nl();

	//Skalowanie od ALARM_MIN do ALARM_MAX
	return tmp;
}

void set_led() {
	if (t_led1==0) {

		//Brak fixa - Szybkie miganie
		if  (!position_valid()) {
			t_led1=100;
			LED1_TOG;
		}

		//Jeœli jest gotowoœæ do zablokowania strefy ale strefa nie jest zablokowana
		else if (((rcfa.status & SET_RTB)==SET_RTB) && !(rcfa.status & SET_BLK)) {
			if (led1_status!=1) {
				t_led1=100;
				LED1_ON;
				led1_status=1;
			}
			else {
				t_led1=700;
				LED1_OFF;
				led1_status=0;
			}
		}

		//W pozosta³ych przypadkach
		else {
			t_led1=10;
			LED1_OFF;
		}


		//Dioda nr 2 - aktywny loger
		if (t_led2==0) {
			if (logger.enabled==1) {
				if (led2_status==0) { t_led2=95; LED2_ON; led2_status=1; }
				else { t_led2=695; LED2_OFF; led2_status=0; }
			}
			else {
				t_led2=90;
				LED2_OFF;
			}
		}
	}
}

void hw_led_blink(uint8_t r) {
	delay_ms(375);
	while (r--) {
		LED1_ON;	delay_ms(250);
		LED1_OFF;	delay_ms(250);
	}
}

void show_init_status() {
	LED1_OFF; LED2_OFF;
	delay_ms(300);

	//1 - Radio
	if (hw.radio) {
		sd_logp(1, &txt54);
	}
	else {
		hw_led_blink(1);
		sd_logp(1, &txt53);
	}

	//2 - GPS
	if (hw.gps) {
		sd_logp(1, &txt57);
	}
	else {
		ini.loggermode = 0;
		hw_led_blink(2);
		sd_logp(1, &txt56);
	}

	//3 - MPL3115A2
	if (hw.mpl) {
		sd_logp(1, &txt51);
	}
	else {
		ini.baro = 0;
		hw_led_blink(3);
		sd_logp(1, &txt52);
	}

	//B³¹d montowania karty
	if (hw.sdmount!=1) hw_led_blink(4);

	//B³¹d odczytu konfiguracji z rcfa.ini
	else if (hw.sdini!=1) hw_led_blink(5);

	//Sygna³ "Wszystko OK"
	if (hw.radio && hw.gps && hw.sdmount && hw.sdini && hw.mpl) {
		LED1_ON; LED2_ON;
		delay_ms(400);
		LED1_OFF; LED2_OFF;
	}

	LED1_OFF; LED2_OFF;
	delay_ms(200);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void tx_loop() {
	//uart_init(UBRR_38400);

	//Inicjalizcje
	LED1_ON;

	//Wykrycie i odczyt konfiguracji z SD
	sd_log_init();
	logger_read_ini();

	//Reset pozycji w EEPROM
	if (ini.test==2) {
		memory_reset();
		sd_logp(1, &txt82);
		system_stop();
	}

	//Inicjalizacja strefy
	rcfa_init();

	//Konfiguracja radia
	radio_init_tx();

	//Odszukanie i konfiguracja modu³u GPS
	gsp_init();
	//uart_init(UBRR_38400);

	//Inicjalizacja wysokoœciomierza
	if (ini.baro) mpl_init();

	//Wynik inicjalizacji
	show_init_status();

	//TEST SYSTEMU
	if (ini.test==1) {
		//Strefa testowa
		memtest(0);

		if (!(hw.sdini) || (ini.iniwspcnt!=AREA_SET_SD)) {
			rcfa.dist = 200;
			rcfa.angle = 60;
			rcfa.sb = 380;
			rcfa.sc = 30;

			//Mt.Everest
			rcfa.P1.lat = 27.988343; rcfa.P1.lon = 86.925226;
			rcfa.P2.lat = 27.988380; rcfa.P2.lon = 86.924628;

			ini.floor = 1;
			ini.iniwspcnt = AREA_SET_SD;
			rcfa_init();
			rcfa.refalt_gps = 8848.0;
		}
		else {
			rcfa_init();
			rcfa.refalt_gps = ini.refalt_gps;
		}

		rcfa.radius = calc_earth_radius(rcfa.P1.lat);

		if (calc_SAB()) {
			rcfa.status |= SET_CAB;

			strcpy_P(gpsData.cdate, PSTR("010116"));
			strcpy_P(gpsData.ctime, PSTR("100000"));
			logger_open();

			logger.buf[0].w3d.w.lat = rcfa.A.lat; logger.buf[0].w3d.w.lon = rcfa.A.lon; logger.buf[0].w3d.alt = rcfa.refalt_gps+(rcfa.h/2);
			strcpy(logger.buf[0].cdate, gpsData.cdate); strcpy(logger.buf[0].ctime, gpsData.ctime);

			strcpy_P(gpsData.ctime, PSTR("101000"));
			logger.buf[1].w3d.w.lat = rcfa.B.lat; logger.buf[1].w3d.w.lon = rcfa.B.lon; logger.buf[1].w3d.alt = logger.buf[0].w3d.alt;
			strcpy(logger.buf[1].cdate, gpsData.cdate); strcpy(logger.buf[1].ctime, gpsData.ctime);

			logger.i = 2;
			logger_write_buf();

			logger.maxspeed = 10.0;
			logger.maxalt = 10.0;
			logger.gztime = 10000;
			logger_close();
		}

		system_stop();
	}
	else if (ini.test==3) {
		//Zrzut pamiêci na kartê SD
		memtest(1);
		system_stop();
	}

	///////////////////////////////////////
	//  Sprawdzenie dostêpnoœci sprzêtu  //
	///////////////////////////////////////
	//Jesli nie ma ani GPS ani Baro
	if ((hw.gps!=1) && (hw.mpl!=1)) {
		//Brak GPS i MPL w trybie vario
		sd_logp(1, &txt76);
		sd_umount();
		system_stop();
	}

	//Jeœli jest tylko barometr
	if ((hw.gps!=1) && (hw.mpl)) {
		//Automatyczne ustawienie trybu wario
		ini.variomode = 2;
		rcfa.status = 0;
		uart_disable();
		sd_logp(1, &txt79);
	}

	//Wys³anie konfiguracji do odbiornika
	txtype = 1;
	txtype_cnt = RESEND_CNT;

	//Init complete
	sd_logp(1, &txt50);


	////////////////////
	//  Pêtla g³ówna  //
	////////////////////
	while (1) {

		//Obs³uga przycisków
		key_check(&btn1_status, &btn1_timer, BTN1_PINPORT, BTN1_PIN);
		key_check(&btn2_status, &btn2_timer, BTN2_PINPORT, BTN2_PIN);

		//Wy³¹czenie trybu debugowania przez przytrzymanie przycisku B
		if ((btn2_status==KEY_DN_LONG_S) && (ini.loglevel>10)) ini.loglevel = 5;

		///////////////////////////////////////
		//	Odczyt danych z GPS i wysokoœci  //
		///////////////////////////////////////
		if (t_getGPSdata==0) {
			t_getGPSdata = GPSDATA_DELAY;

			uint8_t alt_ok=0;
			uint8_t gps_ok=0;

			//Odczyt GPS
			gps_ok = getGPSdata();

			//Logowanie minimalnej i maksymalnej liczby dostêpnych satelit
			if ((ini.loglevel) && (logger.enabled==1)) {
				if (gpsData.numsat < gpsData.numsat_min) gpsData.numsat_min = gpsData.numsat;
				else if (gpsData.numsat > gpsData.numsat_max) gpsData.numsat_max = gpsData.numsat;

				//Logowanie liczby aktywnych satelit gdy aktywny loger GPS
				if ((ini.loglevel>=4) && (gpsData.numsat != gpsData.numsat_prev)) {
					gpsData.numsat_prev = gpsData.numsat;
					sd_log_val(4, "Sat", 0, gpsData.numsat);
				}
			}

			//Wysokoœæ z ciœnieniomierza
			if (hw.mpl) {
				//Odczyt wysokoœci z MPL
				alt_ok = mpl_getAlt(&rcfa.L.alt);
			}
			else if (gps_ok) {
				//Jeœli MPL nie jest dostêpny to wysokoœæ z GPS
				rcfa.L.alt = gpsData.alt;
				alt_ok = 1;
			}

			//Korekta wysokoœci
			if ((ini.floor==1) && (rcfa.L.alt<rcfa.refalt) && (rcfa.refalt_prep==REFALT_READY)) rcfa.L.alt = rcfa.refalt;
			else if (ini.floor>1) rcfa.L.alt = 0;

			//Filtrowanie
			double ret=0;

			//Filtr wspó³rzêdnych
			if (gps_ok) {
				if (ini.loglevel==DEBUG_GPS) sd_log_val(DEBUG_GPS, "Lat1", rcfa.L.w.lat, 0);
				if (ini.filter==1) {
					ret = kalman(rcfa.L.w.lat, &klat.xk, &klat.pk, ini.cr, ini.cq);
					if (klat.ready) rcfa.L.w.lat = ret; else klat.ready = filter_ready(ret, rcfa.L.w.lat, C_READY_PREP);
				}
				else klat.ready = 1;
				if (ini.loglevel==DEBUG_GPS) sd_log_val(DEBUG_GPS, "Lat2", ret, 0);

				if (ini.loglevel==DEBUG_GPS) sd_log_val(DEBUG_GPS, "Lon1", rcfa.L.w.lon, 0);
				if (ini.filter==1) {
					ret = kalman(rcfa.L.w.lon, &klon.xk, &klon.pk, ini.cr, ini.cq);
					if (klon.ready) rcfa.L.w.lon = ret; else klon.ready = filter_ready(ret, rcfa.L.w.lon, C_READY_PREP);
				}
				else klon.ready = 1;
				if (ini.loglevel==DEBUG_GPS) sd_log_val(DEBUG_GPS, "Lon2", ret, 0);
			}

			//Filtr wysokoœci
			if (alt_ok) {
				if (ini.loglevel==DEBUG_ALT) sd_log_val(DEBUG_ALT, "Alt1", rcfa.L.alt, 0);
				if (ini.filter==1) {
					ret = kalman(rcfa.L.alt, &kalt.xk, &kalt.pk, ini.ar, ini.aq);
					if (kalt.ready) rcfa.L.alt = ret; else { kalt.ready = filter_ready(ret, rcfa.L.alt, ALT_READY_PREP); rcfa.refalt_prep=REFALT_CNT; }
				}
				else kalt.ready = 1;
				if (ini.loglevel==DEBUG_ALT) sd_log_val(DEBUG_ALT, "Alt2", ret, 0);
			}


			//Ustalenie g³ównej wysokoœci referencyjnej
			if ((rcfa.refalt_prep!=REFALT_READY) && (alt_ok) && (kalt.ready)) {
				//Jeœli jest barometr
				if ((hw.mpl) || (ini.floor>1)) rcfa.refalt_prep--;
				else {
					//Jeœli nie ma barometru
					if (gpsData.numsat>=ini.refsat) rcfa.refalt_prep--;
				}

				if (rcfa.refalt_prep==REFALT_READY) {
					rcfa.refalt = rcfa.L.alt;
					sd_log_val(2, "Ref.Alt", rcfa.refalt, 0);

					//Znacznik ustalenia wysokoœci referencyjnej
					txtype = 2;
					txtype_cnt = RESEND_CNT;
				}
			}


			//Ustalenie wysokoœci referencyjnej GPS
			if ((rcfa.refalt_gps==REFALTGPS_NOT_SET) && (position_valid()) && (gps_ok) && (alt_ok)) {
				if (ini.refalt_gps!=0) rcfa.refalt_gps = ini.refalt_gps;
				else if (gpsData.numsat>=ini.refsat) rcfa.refalt_gps = gpsData.alt;

				if (rcfa.refalt_gps!=REFALTGPS_NOT_SET) {
					//Korekta g³ównej wysokoœci referencyjnej
					if ((hw.mpl!=1) && (ini.refalt_gps==0.0)) {
						rcfa.refalt = rcfa.refalt_gps;
						sd_log_val(2, "Ref.Alt", rcfa.refalt, 0);
					}
					sd_log_val(2, "Ref.AltGPS", rcfa.refalt_gps, 0);
					sd_log_val(3, "HoG", gpsData.hog, 0);

					if (ini.txpos) {
						//Ustalenie punktu referencyjnego
						rcfa.R.lat = rcfa.L.w.lat;
						rcfa.R.lon = rcfa.L.w.lon;
						sd_log_val(1, "Ref.lat", rcfa.R.lat, 0);
						sd_log_val(1, "Ref.lon", rcfa.R.lon, 0);

						//Wys³anie punktu referencyjnego
						txtype = 3;
						txtype_cnt = RESEND_CNT;
					}
				}
			}


			//Ustalenie danych do ramki NMEA
			if (ini.txpos) {
				if (cnt_nmea_encode==0) {
					if ((position_valid()) && (rcfa.refalt_gps!=REFALTGPS_NOT_SET)) {
						cnt_nmea_encode = 1000/GPSDATA_DELAY;	//Aktualizacja co 1 sek.
						if (nmea.t) nmea.t=0; else nmea.t=1;	//Zmiana znacznika czasu
						nmea_encode();
						nmea.ready = 1;
					}
					else {
						nmea.ready = 0;
					}
				}
				else cnt_nmea_encode--;
			}

			//Sprawdzenie pozycji w strefie
			if ((rcfa.status & SET_BLK) && (position_valid())) rcfa.alarm = check_position();
		}


		/////////////////
		//	Wariometr  //
		/////////////////
		if ((ini.variomode) && (rcfa.refalt_prep==REFALT_READY) && (t_vario==0)) {
			t_vario = VARIO_DELAY;
			rcfa.valarm = calc_valarm();
		}


		/////////////////////////////////////////
		//	Jeœli strefa nie jest zablokowana  //
		/////////////////////////////////////////
		if (!(rcfa.status & SET_BLK) && (position_valid())) {
			//Przycisk 1 - Zapamiêtanie punktu Z (P1)
			if (btn1_status==KEY_UP_SHORT) {
				if (rcfa.status & SET_PREV) rcfa.status=0;
				ini.iniwspcnt = 0;

				//Zapamiêtanie punktu Z
				rcfa.P1.lat = rcfa.L.w.lat;
				rcfa.P1.lon = rcfa.L.w.lon;
				rcfa.refalt = rcfa.L.alt;

				//Oznczenie punktu Z jako ustalonego
				rcfa.status |= SET_P1;

				//Oznaczenie punktów SAB do ponownego przeliczenia
				rcfa.status &= ~SET_CAB;

				//Migniêcie diod¹
				LED1_ON; t_led1 = BLINK_TIME;

				//Znacznik ustalenia punktu P1
				txtype = 4;
				txtype_cnt = RESEND_CNT;

				sd_logp(1, &txt45);
				tag_sab = 0;
			}

			//Przycisk 2 - Zapamiêtanie punktu P (P2)
			if (btn2_status==KEY_UP_SHORT) {
				if (rcfa.status & SET_PREV) rcfa.status=0;
				ini.iniwspcnt = 0;

				//Zapamiêtanie punktu P
				rcfa.P2.lat = rcfa.L.w.lat;
				rcfa.P2.lon = rcfa.L.w.lon;
				rcfa.refalt = rcfa.L.alt;

				//Oznczenie punktu P jako ustalonego
				rcfa.status |= SET_P2;

				//Oznaczenie punktów SAB do ponownego przeliczenia
				rcfa.status &= ~SET_CAB;

				//Migniêcie diod¹
				LED1_ON; t_led1 = BLINK_TIME;

				//Znacznik ustalenia punktu P2
				txtype = 5;
				txtype_cnt = RESEND_CNT;

				sd_logp(1, &txt46);
				tag_sab = 0;
			}

			//Wyliczenie punktów SAB jeœli P1 i P2 s¹ dostêpne
			if (!(rcfa.status & SET_CAB) && ((rcfa.status & SET_P1P2)==SET_P1P2) && (rcfa.refalt_prep==REFALT_READY) && (tag_sab==0)) {

				//Ogranicznik wielokrotnego wyliczania SAB
				tag_sab = 1;

				//Obliczenie radiusa w miejscu pilota
				rcfa.radius = calc_earth_radius(rcfa.P1.lat);

				//Sprawdzenie czy aktualna pozycja pilota nie jest za bardzo oddalona od pozycji odczytanej z pamiêci
				if (calc_dist(rcfa.P1, rcfa.L.w)<=RCFA_MAX_DIST_LP1) {
					//Obliczenie strefy
					if (calc_SAB()) {
						//SAB aktualne
						rcfa.status |= SET_CAB;

						//Wys³anie znacznika strefy gotowej do zablokowania RTB
						if (txtype==0) {
							txtype = 6;
							txtype_cnt = RESEND_CNT;
						}

						//CAB OK | RTB
						sd_logp(1, &txt72);

						//Czas do nastêpnego odczytu GPS
						t_gpsBreak = logger.gpsBreak_time;
					}
					else {
						//"CAB FAILED"
						sd_logp(1, &txt73);
					}
				}
				else {
					sd_logp(1, &txt85);
				}
			}

			//Automatyczna blokada strefy
			if ((rcfa.status & SET_RTB)==SET_RTB) {
				uint8_t autolock=0;
				if ((ini.lockspeed>0) && (gpsData.speed>=kph2knots((double)ini.lockspeed))) autolock=1;

				//D³ugie przytrzymanie przycisku 1 - Zablokowanie strefy
				if ((autolock==1) || (btn1_status==KEY_DN_LONG_S)) {

					//Zapis strefy w EEPROM jeœli jest nowa strefa i nie odczytano wspó³rzêdnych z SD
					if (!(rcfa.status & SET_PREV) && (ini.iniwspcnt!=AREA_SET_SD)) {
						area_save();
						sd_logp(1, &txt47);
					}

					//Migniêcie diod¹
					LED1_ON; t_led1=BLINK_TIME;

					//Wys³anie znacznika zablokowania strefy
					txtype = 7;
					txtype_cnt = RESEND_CNT;

					if (autolock==1) sd_logp(2, &txt74); else sd_logp(2, &txt70);
					rcfa.status |= SET_BLK;
				}
			}
		}


		////////////////////////////////////////////
		//	OBS£UGA WYSY£ANIA DANYCH PRZEZ RADIO  //
		////////////////////////////////////////////
		if ((radio.status==0) && (t_radio_tx==0) && (radio.enabled)) {
			t_radio_tx = RADIO_TX;

			if (txtype > 0) {
				if (txtype==1)		{ radio_set_power(POWER_SEND_CONFIG);	radio_tx_info(INFO_CONFIG);		}
				else if (txtype==2)	{ radio_set_power(POWER_SEND_REFALT);	radio_tx_info(INFO_REFALT_SET);	}
				else if (txtype==3)	{ radio_set_power(POWER_SEND_REFPOS);	radio_tx_refpos();				}
				else if (txtype==4)	{ radio_set_power(POWER_SEND_P1);		radio_tx_info(INFO_P1);			}
				else if (txtype==5)	{ radio_set_power(POWER_SEND_P2);		radio_tx_info(INFO_P2);			}
				else if (txtype==6)	{ radio_set_power(POWER_SEND_RTB);		radio_tx_info(INFO_RTB);		}
				else if (txtype==7)	{ radio_set_power(POWER_SEND_BLK);		radio_tx_info(INFO_BLK);		}
				else txtype = 0;

				if (--txtype_cnt==0) {
					if ((txtype<6) && (rcfa.status & SET_CAB)) { txtype = 6; txtype_cnt = RESEND_CNT; }
					else txtype = 0;
				}
			}
			else {
				//Naprzemienne wysy³anie danych
				if (txtype_cnt==0) {
					//Pe³na moc nadawania
					radio_set_power(POWER_FULL);
					radio_tx_numsat(gpsData.numsat);
				}
				else if ((txtype_cnt==1) && (ini.variomode) && (rcfa.refalt_prep==REFALT_READY)) radio_tx_type_val(TX2_TYPE_VALARM, rcfa.valarm);
				else if ((txtype_cnt==2) && (nmea.ready)) radio_tx_pos(TX3_TYPE_NMEA0, nmea.tab[0]);
				else if ((txtype_cnt==3) && (nmea.ready)) radio_tx_pos(TX3_TYPE_NMEA1, nmea.tab[1]);
				else if ((txtype_cnt==4) && (nmea.ready)) radio_tx_pos(TX3_TYPE_NMEA2, nmea.tab[2]);
				else if (rcfa.status & SET_BLK) radio_tx_type_val(TX2_TYPE_ALARM, rcfa.alarm);
				else if ((ini.variomode) && (rcfa.refalt_prep==REFALT_READY)) radio_tx_type_val(TX2_TYPE_VALARM, rcfa.valarm);
				else radio_tx_numsat(gpsData.numsat);
				//else { t_radio_tx=0; txtype_cnt=6; }

				if (++txtype_cnt>6) txtype_cnt=0;
			}
		}

		//Wys³anie danych zapisanych w FIFO
		radio_tx_payload();


		//////////////
		//	Logger  //
		//////////////
		if (ini.loggermode>0) {
			//Logger tryb manualny; ON|OFF - d³ugie przytrzymanie przycisku B
			if (btn2_status==KEY_DN_LONG_S) {
				//Przestawienie logera na tryb manual
				ini.loggermode = 2;

				//W³¹czenie i wy³aczenie logowania mo¿liwe bez FIXa GPS
				if (logger.enabled==0) {
					sd_logp(1, &txt63);
					logger_open();
				}
				else {
					sd_logp(1, &txt68);
					logger_close();
				}
			}

			//Loger GPS
			if ((logger.enabled==1) && (t_logger_buf==0)) {
				//Logowanie co okreœlony czas
				t_logger_buf = (uint16_t)ini.logspeed;

				if (position_valid()) {
					//Zapamiêtanie wspó³rzêdnych w buforze
					logger.buf[logger.i].w3d.w.lat = rcfa.L.w.lat;
					logger.buf[logger.i].w3d.w.lon = rcfa.L.w.lon;

					if (ini.floor<2) {
						double alt = rcfa.L.alt - rcfa.refalt + rcfa.refalt_gps;
						if ((ini.floor==1) && (alt<rcfa.refalt_gps)) alt = rcfa.refalt_gps;
						logger.buf[logger.i].w3d.alt = alt;
					}
					else if (ini.floor==2) logger.buf[logger.i].w3d.alt = rcfa.refalt_gps;
					else logger.buf[logger.i].w3d.alt = 0;

					//Data i godzina
					strcpy(logger.buf[logger.i].cdate, gpsData.cdate);
					strcpy(logger.buf[logger.i].ctime, gpsData.ctime);

					//Zapamiêtanie maksymalnej wysokoœci
					if (rcfa.L.alt > logger.maxalt) logger.maxalt = rcfa.L.alt;

					//Zapamiêtanie maksymalnej prêdkoœci
					if (gpsData.speed > logger.maxspeed) logger.maxspeed = gpsData.speed;

					logger.i++;
				}

				//Zapis wspó³rzêdnych z bufora na kartê SD
				if (logger.i==SD_MAX_BUF) logger_write_buf();
			}
		} //END logger


		//Obs³uga diod
		if (hw.gps) set_led();

		delay_ms(1);

	} //END while (1)
}

#endif
