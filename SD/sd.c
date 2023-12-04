/*
 * sd.c
 *
 *  Created on: 8 maj 2015
 *      Author: awitczak
 */

#include "../config.h"
#if RADIO_MODE==1

#include "sd.h"

//Liczba znaków w najd³u¿szym stringu (+2)
#define TXT_MAXLEN		64

const char txt0[]  PROGMEM = "\r\n";
const char txt1[]  PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
const char txt2[]  PROGMEM = "<kml xmlns:kml=\"http://www.opengis.net/kml/2.2\"";
const char txt3[]  PROGMEM = " xmlns:gx=\"http://www.google.com/kml/ext/2.2\">";
const char txt4[]  PROGMEM = "<Placemark><styleUrl>air</styleUrl><gx:Track>";
const char txt5[]  PROGMEM = "<visibility>0</visibility>";
const char txt6[]  PROGMEM = "</Folder>";
const char txt7[]  PROGMEM = "</Placemark>";
const char txt8[]  PROGMEM = "Flight plane";
const char txt9[]  PROGMEM = "</name>";
const char txt10[] PROGMEM = "Area center";
const char txt11[] PROGMEM = "Margins";
const char txt12[] PROGMEM = "Flight Area</name>";
const char txt13[] PROGMEM = "Markers</name>";
const char txt14[] PROGMEM = "</gx:Track>";
const char txt15[] PROGMEM = "<Placemark><name>";
const char txt16[] PROGMEM = "<styleUrl>pin</styleUrl><Point>";
const char txt17[] PROGMEM = "</coordinates></Point></Placemark>";
const char txt18[] PROGMEM = "</coordinates></LineString></Placemark>";
const char txt19[] PROGMEM = "</color><width>2</width></LineStyle></Style>";
const char txt20[] PROGMEM = "<Document><name>RCFA";
const char txt21[] PROGMEM = "</Document></kml>";
const char txt22[] PROGMEM = "Fly path</name><open>0</open>";
const char txt23[] PROGMEM = "<Folder><name>";
const char txt24[] PROGMEM = "<when>";
const char txt25[] PROGMEM = "</when><gx:coord>";
const char txt26[] PROGMEM = "</gx:coord>";
const char txt27[] PROGMEM = "<description>RCFA Tracker";
const char txt28[] PROGMEM = "<Style id=\"";
const char txt29[] PROGMEM = "pin\">";
const char txt30[] PROGMEM = "air\">";
const char txt31[] PROGMEM = "<IconStyle><Icon><href>";
const char txt32[] PROGMEM = "http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png";
const char txt33[] PROGMEM = "http://maps.google.com/mapfiles/kml/shapes/airports.png";
const char txt34[] PROGMEM = "</href></Icon></IconStyle>";
const char txt35[] PROGMEM = "</Style>";
const char txt36[] PROGMEM = "<LineStyle><color>ff0000ff</color><width>3</width></LineStyle>";
const char txt37[] PROGMEM = "</description>";
const char txt38[] PROGMEM = "<altitudeMode>absolute</altitudeMode>";
const char txt39[] PROGMEM = "<coordinates>";
const char txt40[] PROGMEM = "<extrude>1</extrude>";
const char txt41[] PROGMEM = "<styleUrl>";
const char txt42[] PROGMEM = "</styleUrl><LineString>";
const char txt43[] PROGMEM = "\"><LineStyle><color>";
const char txt44[] PROGMEM = "--- RC Flight Assist ---";
const char txt45[] PROGMEM = "P1 OK";
const char txt46[] PROGMEM = "P2 OK";
const char txt47[] PROGMEM = "Area Saved";
const char txt48[] PROGMEM = "Area loaded from memory";
const char txt49[] PROGMEM = "Area loaded from rcfa.ini";
const char txt50[] PROGMEM = "Init sequence complete";
const char txt51[] PROGMEM = "Altimeter OK";
const char txt52[] PROGMEM = "Altimeter disconnected";
const char txt53[] PROGMEM = "Radio init FAILED";
const char txt54[] PROGMEM = "Radio init OK";
const char txt55[] PROGMEM = "Radio write FAILED";
const char txt56[] PROGMEM = "GPS init FAILED";
const char txt57[] PROGMEM = "GPS init OK";
const char txt58[] PROGMEM = "INI read FAILED";
const char txt59[] PROGMEM = "Jumper off";
const char txt60[] PROGMEM = "Jumper 1-2";
const char txt61[] PROGMEM = "Jumper 2-3";
const char txt62[] PROGMEM = "Logger auto start";
const char txt63[] PROGMEM = "Logger manual start";
const char txt64[] PROGMEM = "KML open OK";
const char txt65[] PROGMEM = "KML open FAILED";
const char txt66[] PROGMEM = "INI read OK";
const char txt67[] PROGMEM = "Logger auto close";
const char txt68[] PROGMEM = "Logger manual close";
const char txt69[] PROGMEM = "Logger close FAILED";
const char txt70[] PROGMEM = "Manual Area Lock";
const char txt71[] PROGMEM = "Invalid P1<->P2 distance (1-500m)";
const char txt72[] PROGMEM = "Area Ready To Lock";
const char txt73[] PROGMEM = "CAB FAILED";
const char txt74[] PROGMEM = "Automatic Area Lock";
const char txt75[] PROGMEM = "Invalid P1 or P2";
const char txt76[] PROGMEM = "GPS or MPL not available";
const char txt77[] PROGMEM = "GPS valid";
const char txt78[] PROGMEM = "GPS fix lost";
const char txt79[] PROGMEM = "GPS not available, variomode enabled";
const char txt80[] PROGMEM = "GPS acc LOW";
const char txt81[] PROGMEM = "GPS acc NORM";
const char txt82[] PROGMEM = "Memory reset";
//const char txt83[] PROGMEM = "FHSS Enabled";
const char txt84[] PROGMEM = "Calculating CAB";
const char txt85[] PROGMEM = "Position too far away from Flight Area";
const char txt86[] PROGMEM = "System Stop";
const char txt87[] PROGMEM = "GPS module lost";

//Wy³¹czone z memtest
const char info1[] PROGMEM = "--- TEST START ---";
const char info2[] PROGMEM = FIRMWARE_VER;
//const char info3[] PROGMEM = "SYSTEM HACKED ]:-<";
const char info4[] PROGMEM = "--- TEST END ---";
const char info5[] PROGMEM = "FIRMWARE TEST OK";
const char info6[] PROGMEM = "FIRMWARE TEST FAILED";

FATFS FatFs;	//FatFs
FIL Fil;		//Plik KML
FIL Flog;		//Plik log

volatile uint16_t t_logger_buf;		//Timer zapisu danych do bufora
volatile uint16_t t_sd_proc;		//Procedura sprawdzania dostêpnoœci karty co 10ms

//G³ówna zmienna Loggera
s_logger logger;

//Bufor do odczytu z EEPROM
char strbuf[TXT_MAXLEN];

const PGM_P memtab[] PROGMEM = {
	txt1,txt2,txt3,txt4,txt5,txt6,txt7,txt8,txt9,txt10,txt11,txt12,txt13,txt14,txt15,txt16,txt17,txt18,txt19,txt20,txt21,txt22,
	txt23,txt24,txt25,txt26,txt27,txt28,txt29,txt30,txt31,txt32,txt33,txt34,txt35,txt36,txt37,txt38,txt39,txt40,txt41,txt42,txt43,
	txt44,txt45,txt46,txt47,txt48,txt49,txt50,txt51,txt52,txt53,txt54,txt55,txt56,txt57,txt58,txt59,txt60,txt61,txt62,txt63,txt64,
	txt65,txt66,txt67,txt68,txt69,txt70,txt71,txt72,txt73,txt74,txt75,txt76,txt77,txt78,txt79,txt80,txt81,txt82,txt84,txt85,txt86
};

void memtest(uint8_t print) {
	uint8_t chk = 1;
	uint8_t j;

	if (print) sd_logp(0, &info1);
	for (uint8_t i=0; i<NUM_ELEMENTS(memtab); i++) {
		strcpy_P(strbuf, (PGM_P)pgm_read_word(&(memtab[i])));
		if (print) sd_log(0, strbuf);
		j = 0;
		while (strbuf[j]) {
			if ((strbuf[j]!='\r') && (strbuf[j]!='\n') && (strbuf[j]<32 || strbuf[j]>126)) chk=0;
			j++;
		}
	}
	if (print) sd_logp(0, &info4);

	if (chk) sd_logp(1, &info5);
	else sd_logp(1, &info6);

	LED1_OFF; LED2_OFF;
}

FRESULT sd_mount() {
	uint8_t i=0;
	FRESULT ret;

	do {
		ret = f_mount(&FatFs, "", 1);
	} while ((ret!=FR_OK) && (++i<SD_ITER));

	return ret;
}

FRESULT sd_umount() {
	return f_mount(NULL, "", 1);
}

FRESULT open_file(FIL *f, const char *fname, BYTE mode) {
	uint8_t i=0;
	FRESULT ret;

	do {
		ret = f_open(f, fname, mode);
		i++;
	} while ((ret!=FR_OK)&&(i<SD_ITER));

	return ret;
}

FRESULT open_append(FIL *f, const char *fname) {
	FRESULT ret;

	ret = open_file(f, fname, FA_WRITE | FA_OPEN_EXISTING);

	//Przejœcie na koniec pliku
	if (ret==FR_OK) ret = f_lseek(f, f_size(f));

	return ret;
}

void f_putp(void *adr, FIL *f) {
	char c;
	uint8_t i = 0;
	uint8_t *ptr = (uint8_t*)adr;

	while ((c=pgm_read_byte(ptr++))) strbuf[i++] = c;
	strbuf[i] = '\0';

	f_puts(strbuf, f);
}

uint8_t sd_log_init() {
	if (sd_mount()==FR_OK) {
		//Montowanie powiod³o siê
		hw.sdmount = 1;

		if (open_append(&Flog, FNAME_LOG)==FR_OK) {
			//Udany dostêp do pliku log
			gpsData.ctime[0] = ',';
			hw.sdlog = 1;
			f_close(&Flog);
			return 1;
		}
	}
	return 0;
}

uint8_t sd_log_open(uint8_t lv) {
	if ((sd_mount()==FR_OK) && (open_append(&Flog, FNAME_LOG)==FR_OK)) {
		char str[4];

		if (lv>0){
			itoa(lv, str, 10);
			strcat(str, "|\0");
			f_puts(str, &Flog);
		}

		if (gpsData.ctime[0]!=',') {
			f_puts(gpsData.ctime, &Flog);
			f_puts("|", &Flog);
		}
		return 1;

	}
	return 0;
}

void sd_log(uint8_t lv, char *str) {
	if (hw.sdlog!=1) return;
	if (lv>ini.loglevel) return;

	if (sd_log_open(lv)) {
		LED2_ON;
		f_puts(str, &Flog);
		f_putp(&txt0, &Flog);
		f_close(&Flog);
		LED2_OFF;
	}
}

void sd_log_val(uint8_t lv, char *s, double vd, uint16_t vi) {
	if (hw.sdlog!=1) return;
	if (lv>ini.loglevel) return;

	char str[20];
	char line[30];

	if (vd!=0.0) dtostrf(vd,1,6,str);
	else utoa(vi,str,10);

	strcpy(line, s);
	strcat(line, "=");
	strcat(line, str);
	strcat(line, "\0");
	sd_log(lv, line);
}

void sd_logp(uint8_t lv, void *adr) {
	if (hw.sdlog!=1) return;
	if (lv>ini.loglevel) return;

	if (sd_log_open(lv)) {
		f_putp(adr, &Flog);
		f_putp(&txt0, &Flog);
		f_close(&Flog);
	}
}

char *format_date(char *s) {
	//s = ddmmyy\0
	static char d[11];

	#if 0
	//Format dd/mm/yyyy
	d[0]=s[0]; d[1]=s[1]; d[2]='/'; d[3]=s[2]; d[4]=s[3]; d[5]='/'; d[6]='2'; d[7]='0'; d[8]=s[4]; d[9]=s[5]; d[10]='\0';
	#else
	//Format yyyy-mm-dd
	d[0]='2'; d[1]='0'; d[2]=s[4]; d[3]=s[5]; d[4]='-'; d[5]=s[2]; d[6]=s[3]; d[7]='-'; d[8]=s[0]; d[9]=s[1]; d[10]='\0';
	#endif

	return d;
}

char *format_time(char *s, uint8_t utc) {
	//s = hhmmss\0
	//d = hh:mm:ss\0
	static char d[9];
	char str[3];

	if (utc==TIME_UTC) {
		d[0] = s[0];
		d[1] = s[1];
	}
	else {
		//Uwzglêdnienie strefy czasowej
		int8_t h = (s[0]-48)*10 + (s[1]-48);
		h += ini.tzone;
		if (h>23) h-=24;
		else if (h<0) h+=24;

		itoa(h,str,10);
		if (h<10) { d[0] = '0'; d[1] = str[0]; }
		else { d[0] = str[0]; d[1] = str[1]; }
	}

	//Formatowanie mm:ss
	d[2]=':'; d[3]=s[2]; d[4]=s[3]; d[5]=':'; d[6]=s[4]; d[7]=s[5]; d[8]='\0';

	return d;
}

//Formatowanie Flight Time
char *format_ft(uint16_t sec) {
	//sec - liczba sekund
	static char d[15];
	char str[5];
	uint16_t h,m,s;

	strcpy(d,"");

	//Godziny
	h = sec / 3600;
	if (h>0) {
		utoa(h,str,10);
		strcat(d,str);
		strcat(d,"h ");
	}

	//Minuty
	m = (sec / 60) % 60;
	if (h>0 || m>0) {
		utoa(m,str,10);
		strcpy(d,str);
		strcat(d,"m ");
	}

	//Sekundy
	s = sec % 60;
	utoa(s,str,10);
	strcat(d,str);
	strcat(d,"s\0");

	return d;
}

void logger_write_coords(s_wsp *c, double h, s_logbuf *buf) {
	static char prev_coord[30];
	char coord[30];
	char str[25];

	//Lon
	dtostrf(c->lon,1,6,str);
	strcpy(coord, str);
	strcat(coord, ",");

	//Lat
	dtostrf(c->lat,1,6,str);
	strcat(coord, str);
	strcat(coord, ",");

	//Alt
	dtostrf(h,1,2,str);
	strcat(coord, str);
	strcat(coord, "\0");

	//Zapis jeœli nowe wspó³rzêdne s¹ ró¿ne od poprzednich
	if (strcmp(coord, prev_coord)!=0) {
		strcpy(prev_coord, coord);

		//Zapis œcie¿ki
		if (buf) {
			strcpy(str, format_date(buf->cdate));
			strcat(str, "T");
			strcat(str, format_time(buf->ctime, TIME_UTC));
			strcat(str, "Z");

			f_putp(&txt24, &Fil);
			f_puts(str, &Fil);
			f_putp(&txt25, &Fil);
			f_puts((char *)coord, &Fil);
			f_putp(&txt26, &Fil);
			f_putp(&txt0, &Fil);
		}
		//Zapis punktu
		else {
			strcat(coord," \0");	//Rozdzielenie spacj¹
			f_puts((char *)coord, &Fil);
		}
	}
}

void write_point(const char *c, s_wsp *wsp, double alt, uint8_t mode) {
	f_putp(&txt15, &Fil);
	f_puts(c, &Fil);
	f_putp(&txt9, &Fil);

	if (mode & GE_NOT_VISIBLE) f_putp(&txt5, &Fil);

	f_putp(&txt16, &Fil);

	if (mode & GE_EXTRUDE) f_putp(&txt40, &Fil);
	if (mode & GE_ALT_ABSOLUTE) f_putp(&txt38, &Fil);

	f_putp(&txt39, &Fil);
	logger_write_coords(wsp, alt, NULL);
	f_putp(&txt17, &Fil);
	f_putp(&txt0, &Fil);
}

void write_line(const char *c, s_wsp *a, double ha, s_wsp *b, double hb, uint8_t mode) {
	f_putp(&txt15, &Fil);
	f_puts(c, &Fil);
	f_putp(&txt9, &Fil);

	if (mode & GE_NOT_VISIBLE) f_putp(&txt5, &Fil);

	//Style
	f_putp(&txt41, &Fil);
	if (mode & GE_STYLE_PL) f_puts("pl", &Fil);
	else if (mode & GE_STYLE_LW) f_puts("lw", &Fil);
	else if (mode & GE_STYLE_AW) f_puts("aw", &Fil);
	f_putp(&txt42, &Fil);

	//Odniesienie wysokoœci
	if (mode & GE_EXTRUDE) f_putp(&txt40, &Fil);
	//if (mode & GE_ALT_REL2GND) f_putp(&txt4, &Fil);
	if (mode & GE_ALT_ABSOLUTE) f_putp(&txt38, &Fil);

	f_putp(&txt39, &Fil);
	logger_write_coords(a, ha, NULL);
	logger_write_coords(b, hb, NULL);
	f_putp(&txt18, &Fil);
	f_putp(&txt0, &Fil);
}

void write_style(char* n, char* v) {
	f_putp(&txt28, &Fil);
	f_puts(n, &Fil);
	f_putp(&txt43, &Fil);
	f_puts(v, &Fil);
	f_putp(&txt19, &Fil);
	f_putp(&txt0, &Fil);
}

void logger_read_ini() {
	char line[30];
	char *p;
	double *d;
	int8_t *i8;
	uint8_t *ui8;
	uint16_t *ui16;

	//Wy³¹czenie RX
	uart_disable_rx();

	//Inicjacja zmiennych - Ustawienia domyœlne
	rcfa.dist = F3A_DEF_D;
	rcfa.angle = F3A_DEF_K;
	rcfa.sb = F3A_DEF_SB;
	rcfa.sc = F3A_DEF_SC;
	rcfa.margin = F3A_DEF_MARGIN;
	rcfa.P1.lat = 0;
	rcfa.P1.lon = 0;
	rcfa.P2.lat = 0;
	rcfa.P2.lon = 0;

	ini.filter = INI_DEF_KALMAN;
	ini.cr = INI_DEF_CR;
	ini.cq = INI_DEF_CQ;
	ini.ar = INI_DEF_AR;
	ini.aq = INI_DEF_AQ;
	ini.vr = INI_DEF_VR;
	ini.vq = INI_DEF_VQ;

	ini.baro = INI_DEF_BARO;
	ini.hlimit = INI_DEF_HLIMIT;
	ini.hlowlimit = INI_DEF_HLOWLIMIT;
	ini.lockspeed = INI_DEF_LOCKSPEED;
	ini.vol = INI_DEF_VOL;
	ini.lowacc = INI_DEF_LOWACC;
	ini.loglevel = INI_DEF_LOGLEVEL;
	ini.variomode = INI_DEF_VARIOMODE;
	ini.hspmin = INI_DEF_HSPMIN;
	ini.hspmax = INI_DEF_HSPMAX;
	ini.radio = INI_DEF_RADIO;
	ini.test = INI_DEF_TEST;
	ini.txpos = INI_DEF_TXPOS;
	ini.refsat = INI_DEF_REFSAT;
	ini.refalt_gps = INI_DEF_REFALT;
	ini.loggermode = INI_DEF_MODE;
	ini.logspeed = INI_DEF_LOGSPEED;
	ini.tzone = INI_DEF_TZONE;
	ini.gpshz = INI_DEF_GPSHZ;
	ini.floor = INI_DEF_FLOOR;

	//Licznik wspó³rzêdnych odczytanych z ini
	ini.iniwspcnt = 0;

	//Montowanie karty
	if (sd_mount()==FR_OK) {
		//Jeœli plik otwarty poprawnie
		if (open_file(&Fil, FNAME_INI, FA_READ)==FR_OK) {
			//uart_puts("Fopen read OK\r\n");
			while (f_gets(line, sizeof line, &Fil)) {
				if ((line[0]!='#') && (line[0]!=' ') && (line[0]!='\0') && (line[0]!='\r') && (line[0]!='\n')) {
					//Odczyt komendy
					p = strtok((char*)line,"=");

					//Zerowanie wskaŸników
					d = NULL;
					i8 = NULL;
					ui8 = NULL;
					ui16 = NULL;

					if (strcmp(p,INI_D)==0)					d = &rcfa.dist;
					else if (strcmp(p,INI_K)==0)			d = &rcfa.angle;
					else if (strcmp(p,INI_SB)==0)			d = &rcfa.sb;
					else if (strcmp(p,INI_SC)==0)			d = &rcfa.sc;
					else if (strcmp(p,INI_HSPMIN)==0)		d = &ini.hspmin;
					else if (strcmp(p,INI_HSPMAX)==0)		d = &ini.hspmax;
					else if (strcmp(p,INI_CR)==0)			d = &ini.cr;
					else if (strcmp(p,INI_CQ)==0)			d = &ini.cq;
					else if (strcmp(p,INI_AR)==0)			d = &ini.ar;
					else if (strcmp(p,INI_AQ)==0)			d = &ini.aq;
					else if (strcmp(p,INI_VR)==0)			d = &ini.vr;
					else if (strcmp(p,INI_VQ)==0)			d = &ini.vq;
					else if (strcmp(p,INI_LOGSPEED)==0)		d = &ini.logspeed;
					else if (strcmp(p,INI_REFALT)==0)		d = &ini.refalt_gps;
					else if (strcmp(p,INI_Z_LAT)==0)		{ d = &rcfa.P1.lat;	ini.iniwspcnt++; }
					else if (strcmp(p,INI_Z_LON)==0)		{ d = &rcfa.P1.lon;	ini.iniwspcnt++; }
					else if (strcmp(p,INI_P_LAT)==0)		{ d = &rcfa.P2.lat;	ini.iniwspcnt++; }
					else if (strcmp(p,INI_P_LON)==0)		{ d = &rcfa.P2.lon;	ini.iniwspcnt++; }

					else if (strcmp(p,INI_TZONE)==0)		i8  = &ini.tzone;
					else if (strcmp(p,INI_MARGIN)==0)		ui8 = &rcfa.margin;
					else if (strcmp(p,INI_VOL)==0)			ui8 = &ini.vol;
					else if (strcmp(p,INI_LOWACC)==0)		ui8 = &ini.lowacc;
					else if (strcmp(p,INI_LOGMODE)==0)		ui8 = &ini.loggermode;
					else if (strcmp(p,INI_GPSHZ)==0)		ui8 = &ini.gpshz;
					else if (strcmp(p,INI_FLOOR)==0)		ui8 = &ini.floor;
					else if (strcmp(p,INI_FILTER)==0)		ui8 = &ini.filter;
					else if (strcmp(p,INI_RADIO)==0)		ui8 = &ini.radio;
					else if (strcmp(p,INI_BARO)==0)			ui8 = &ini.baro;
					else if (strcmp(p,INI_LSPEED)==0)		ui8 = &ini.lockspeed;
					else if (strcmp(p,INI_DEBUG)==0)		ui8 = &ini.loglevel;
					else if (strcmp(p,INI_VARIOMODE)==0)	ui8 = &ini.variomode;
					else if (strcmp(p,INI_TEST)==0)			ui8 = &ini.test;
					else if (strcmp(p,INI_TXPOS)==0)		ui8 = &ini.txpos;
					else if (strcmp(p,INI_REFSAT)==0)		ui8 = &ini.refsat;
					else if (strcmp(p,INI_HLIMIT)==0)		ui16 = &ini.hlimit;
					else if (strcmp(p,INI_HLOWLIMIT)==0)	ui16 = &ini.hlowlimit;

					//Odczyt wartoœci
					p = strtok(NULL,"=");
					if (d) *d = strtod(p,NULL);
					else if (i8) *i8 = (int8_t)atoi(p);
					else if (ui8) *ui8 = (uint8_t)atoi(p);
					else if (ui16) *ui16 = (uint16_t)atoi(p);
				}
			}
			f_close(&Fil);

			//Log level
			if (ini.test) ini.loglevel = 3;
			else {
				if (ini.loglevel<INI_DEBUG_MIN) ini.loglevel=INI_DEBUG_MIN;
				if (ini.loglevel>INI_DEBUG_MAX) ini.loglevel=INI_DEBUG_MAX;
			}

			/////////////////
			//  WALIDACJE  //
			/////////////////

			//Dystans
			if (rcfa.dist<F3A_D_MIN) rcfa.dist=F3A_D_MIN;
			if (rcfa.dist>F3A_D_MAX) rcfa.dist=F3A_D_MAX;

			//K¹t
			if (rcfa.angle<F3A_K_MIN) rcfa.angle=F3A_K_MIN;
			if (rcfa.angle>F3A_K_MAX) rcfa.angle=F3A_K_MAX;

			//Szerokoœæ boxa
			if (rcfa.sb<F3A_SB_MIN) rcfa.sb=F3A_SB_MIN;
			if (rcfa.sb>F3A_SB_MAX) rcfa.sb=F3A_SB_MAX;

			//Szerokoœæ linii lotu
			if (rcfa.sc<F3A_SC_MIN) rcfa.sc=F3A_SC_MIN;
			if (rcfa.sc>rcfa.sb) rcfa.sc=rcfa.sb;

			//Alarm graniczny 'margin'
			if (rcfa.margin>1) rcfa.margin=F3A_DEF_MARGIN;

			//Ogranicznik wysokoœci strefy
			if (ini.hlimit>INI_HLIMIT_MAX) ini.hlimit=INI_HLIMIT_MAX;
			if (ini.hlowlimit>INI_HLIMIT_MAX) ini.hlowlimit=INI_HLIMIT_MAX;

			//W³¹cznik modu³u radiowego
			if (ini.radio>1) ini.radio=INI_DEF_RADIO;

			//W³¹cznik wariometru
			if (ini.variomode>2) ini.variomode=INI_DEF_VARIOMODE;

			//W³¹cznik nadawania nmea - jednoczeœnie liczba satelit, przy których ustawiana jest wysokoœæ referencyjna z GPS
			if (ini.txpos>1) ini.txpos=INI_DEF_TXPOS;
			if (ini.radio==0) ini.txpos=0;

			//Liczba satelitów do okreœlenia wysokoœci referencyjnej
			if (ini.refsat<GPS_REFSAT_MIN) ini.refsat=GPS_REFSAT_MIN;
			if (ini.refsat>GPS_REFSAT_MAX) ini.refsat=GPS_REFSAT_MAX;

			//Wysokoœæ referencyjna
			if (ini.refalt_gps<GPS_MIN_ALT) ini.refalt_gps=GPS_MIN_ALT;
			if (ini.refalt_gps>GPS_MAX_ALT) ini.refalt_gps=GPS_MAX_ALT;

			//Ustawienie loggera
			if (ini.loggermode>2) ini.loggermode=INI_DEF_MODE;

			//Prêdkoœæ odczytu GPS - dopuszczalne wartoœci 1,5,10 Hz
			if ((ini.gpshz!=1) && (ini.gpshz!=5) && (ini.gpshz!=10)) ini.gpshz=INI_DEF_GPSHZ;

			//Czêstotliwoœæ zapisu wspó³rzêdnych w pliku tymczasowym
			if (ini.logspeed<LOGSPEED_MIN) ini.logspeed=LOGSPEED_MIN;
			if (ini.logspeed>LOGSPEED_MAX) ini.logspeed=LOGSPEED_MAX;

			//Tryb floor
			if (ini.floor>3) ini.floor=INI_DEF_FLOOR;
			if (ini.floor>1) { ini.baro=0; ini.variomode=0; }

			//Strefa czasowa
			if ((ini.tzone<TZONE_MIN)||(ini.tzone>TZONE_MAX)) ini.tzone=INI_DEF_TZONE;

			//Filtr kalmana
			if (ini.filter>1) ini.filter=INI_DEF_KALMAN;

			//W³¹cznik wysokoœciomierza
			if (ini.baro>1) ini.baro=INI_DEF_BARO;

			//Autolock
			if (ini.lockspeed>100) ini.lockspeed=INI_DEF_LOCKSPEED;

			//Zerowanie wspó³rzêdnych P1 i P2 jeœli s¹ nieprawid³owe lub nie odczytano kompletu 4 wartoœci z SD
			if ((chk_angles(rcfa.P1)==0) || (chk_angles(rcfa.P2)==0) || (ini.iniwspcnt!=AREA_SET_SD)) {
				rcfa.P1.lat = 0;
				rcfa.P1.lon = 0;
				rcfa.P2.lat = 0;
				rcfa.P2.lon = 0;
			}

			//G³oœnoœæ
			//if (ini.vol<INI_VOL_MIN) ini.vol=INI_VOL_MIN;
			if (ini.vol>INI_VOL_MAX) ini.vol=INI_VOL_MAX;

			//Liczba satelit uznawanych za s³aby sygna³
			//if (ini.lowacc<INI_SAT_MIN) ini.lowacc=INI_SAT_MIN;
			if (ini.lowacc>INI_SAT_MAX) ini.lowacc=INI_SAT_MAX;

			//Ustawienia wariometru
			if (ini.hspmin<INI_HSP_MIN) ini.hspmin=INI_HSP_MIN;
			if (ini.hspmin>INI_HSP_MAX) ini.hspmin=INI_HSP_MAX;
			if (ini.hspmax<INI_HSP_MIN) ini.hspmax=INI_HSP_MIN;
			if (ini.hspmax>INI_HSP_MAX) ini.hspmax=INI_HSP_MAX;
			if (ini.hspmax<ini.hspmin) ini.hspmax=ini.hspmin;

			//Wy³¹czenie loggera w trybie debugowania
			if (ini.loglevel>10) ini.loggermode=0;

			/////////////////
			//  LOGOWANIE  //
			/////////////////

			sd_logp(0, &txt44);
			sd_logp(0, &info2);
			sd_log_val(1, INI_DEBUG, 0, ini.loglevel);
			sd_log_val(1, INI_RADIO, 0, ini.radio);
			sd_log_val(1, INI_BARO, 0, ini.baro);
			sd_log_val(1, INI_D, rcfa.dist, 0);
			sd_log_val(1, INI_K, rcfa.angle, 0);
			sd_log_val(1, INI_SB, rcfa.sb, 0);
			sd_log_val(1, INI_SC, rcfa.sc, 0);
			sd_log_val(1, INI_MARGIN, 0, rcfa.margin);
			sd_log_val(1, INI_HLIMIT, 0, ini.hlimit);
			sd_log_val(1, INI_HLOWLIMIT, 0, ini.hlowlimit);
			sd_log_val(1, INI_VARIOMODE, 0, ini.variomode);
			sd_log_val(1, INI_HSPMIN, ini.hspmin, 0);
			sd_log_val(1, INI_HSPMAX, ini.hspmax, 0);
			sd_log_val(1, INI_TXPOS, 0, ini.txpos);
			sd_log_val(1, INI_REFSAT, 0, ini.refsat);
			sd_log_val(1, INI_REFALT, ini.refalt_gps, 0);
			sd_log_val(1, INI_GPSHZ, 0, ini.gpshz);
			sd_log_val(1, INI_LOGMODE, 0, ini.loggermode);
			sd_log_val(1, INI_LOGSPEED, ini.logspeed, 0);
			sd_log_val(1, INI_FLOOR, 0, ini.floor);
			sd_log_val(1, INI_TZONE, 0, ini.tzone);
			sd_log_val(1, INI_FILTER, 0, ini.filter);
			sd_log_val(1, INI_LSPEED, 0, ini.lockspeed);

			if (ini.iniwspcnt==AREA_SET_SD) {
				sd_log_val(1, INI_Z_LAT, rcfa.P1.lat, 0);
				sd_log_val(1, INI_Z_LON, rcfa.P1.lon, 0);
				sd_log_val(1, INI_P_LAT, rcfa.P2.lat, 0);
				sd_log_val(1, INI_P_LON, rcfa.P2.lon, 0);
			}

			sd_log_val(1, INI_VOL, 0, ini.vol);
			sd_log_val(1, INI_LOWACC, 0, ini.lowacc);

			//Ukryte
			if (ini.cr!=INI_DEF_CR) sd_log_val(3, INI_CR, ini.cr, 0);
			if (ini.cq!=INI_DEF_CQ) sd_log_val(3, INI_CQ, ini.cq, 0);
			if (ini.ar!=INI_DEF_AR) sd_log_val(3, INI_AR, ini.ar, 0);
			if (ini.aq!=INI_DEF_AQ) sd_log_val(3, INI_AQ, ini.aq, 0);
			if (ini.vr!=INI_DEF_VR) sd_log_val(3, INI_VR, ini.vr, 0);
			if (ini.vq!=INI_DEF_VQ) sd_log_val(3, INI_VQ, ini.vq, 0);
			if (ini.test!=INI_DEF_TEST) sd_log_val(3, INI_TEST, 0, ini.test);

			//Zamiana na ms
			ini.logspeed = ini.logspeed*(double)1000.0;

			//Oznaczenie kongfiguracji jako poporawnej
			hw.sdini = 1;
			sd_logp(1, &txt66);

			f_close(&Fil);
		}
		else {
			sd_logp(1, &txt58);
		}
	}

	//Logger pocz¹tkowo wy³¹czony
	logger.enabled = 0;
	//logger.gpsBreak_time = (double)GPS_MAX_BREAK*(1.0/(double)ini.gpshz);
	logger.gpsBreak_time = (double)GPS_BREAK;

	#if 0
	//Test
	uart_puts("d="); uart_putd(rcfa.dist,1);
	uart_puts("\r\nk="); uart_putd(rcfa.angle,1);
	uart_puts("\r\nsb="); uart_putd(rcfa.box,1);
	uart_puts("\r\nsc="); uart_putd(rcfa.line,1);
	uart_puts("\r\nag="); uart_putd(rcfa.ag,1);
	uart_puts("\r\nlog mode="); uart_putl(logger.mode,10);
	uart_puts("\r\nlog zone="); uart_putl(logger.tzone,10);
	uart_nl();
	#endif

	//W³¹czenie RX
	uart_enable_rx();

	spi_disable();
}

void logger_open() {
	LED2_ON;
	t_led2=BLINK_TIME;

	//Wy³¹czenie RX
	uart_disable_rx();

	//Montowanie karty do koñca trwania logowania
	if (sd_mount()==FR_OK) {

		/* 0 - Z³o¿enie nazwy pliku f3a_ddmmyy_hhmmss.kml - wymagana obs³uga d³ugich nazw w FatFS */
		/* 1 - Nazwa pliku typu: rcfaX.kml*/
		#if 1
		//Okreœlenie nazwy pliku z kolejnym numerem
		FILINFO fno;
		char fnstr[5];
		uint16_t fnum = 1;
		do {
			itoa(fnum, fnstr, 10);
			strcpy(logger.fname,FNAME_KML);
			strcat(logger.fname,fnstr);
			strcat(logger.fname,".kml\0");
			//uart_puts(logger.fname); uart_nl();
			fnum++;
		} while ((fnum<=SD_MAX_FNUM) && (f_stat(logger.fname, &fno)==FR_OK));
		#else
		strcpy(logger.fname, "f3a_");
		strcat(logger.fname, gpsData.date);
		strcat(logger.fname, "_");
		strcat(logger.fname, gpsData.date);
		strcat(logger.fname, ".kml\0");
		#endif

		//Zalogowanie nazwy pliku
		sd_log(1,logger.fname);
		LED2_ON;

		//Otwarcie pliku
		if ((fnum<=SD_MAX_FNUM) && (open_file(&Fil, (char *)logger.fname, FA_WRITE | FA_CREATE_ALWAYS)==FR_OK)) {
		//if (open_file((char *)logger.fname, FA_WRITE | FA_CREATE_ALWAYS)==FR_OK) {
			//uart_puts("Fopen OK...Writing...\r\n");

			//Nag³ówek
			f_putp(&txt1, &Fil); f_putp(&txt0, &Fil);
			f_putp(&txt2, &Fil); f_putp(&txt3, &Fil); f_putp(&txt0, &Fil);
			//f_sync(&Fil);

			//Document name
			f_putp(&txt20, &Fil);
			//Nazwa|Data|Godzina
			f_puts(fnstr, &Fil);
			f_puts("|", &Fil);
			f_puts(format_date(gpsData.cdate), &Fil);
			f_puts("|", &Fil);
			f_puts(format_time(gpsData.ctime, TIME_TZONE), &Fil);
			//END Document name
			f_putp(&txt9, &Fil);
			f_putp(&txt0, &Fil);
			//f_sync(&Fil);

			//Style
			write_style("pl","ff00ffff");
			write_style("lw","ff00ff00");
			write_style("aw","ffff0000");
			f_putp(&txt28, &Fil); f_putp(&txt29, &Fil); f_putp(&txt31, &Fil); f_putp(&txt32, &Fil); f_putp(&txt34, &Fil); f_putp(&txt35, &Fil); f_putp(&txt0, &Fil);
			f_putp(&txt28, &Fil); f_putp(&txt30, &Fil); f_putp(&txt31, &Fil); f_putp(&txt33, &Fil); f_putp(&txt34, &Fil); f_putp(&txt36, &Fil); f_putp(&txt35, &Fil); f_putp(&txt0, &Fil);
			f_sync(&Fil);

			//Jeœli s¹ dostêpne punkty P1 P2 SAB to rysowanie ramki
			if ((rcfa.status & SET_RTB)==SET_RTB) {
				double refhb = 0;
				double refht = 0;
				uint8_t altmode;

				if (ini.floor<2) {
					altmode = GE_ALT_ABSOLUTE;
					refhb = rcfa.refalt_gps;
					refht = rcfa.refalt_gps+rcfa.h;
				}
				else if (ini.floor==2) {
					altmode = GE_ALT_ABSOLUTE;
					refhb = rcfa.refalt_gps;
				}
				else {
					altmode = 0;
				}

				//Folder F3A Area
				f_putp(&txt23, &Fil); f_putp(&txt12, &Fil); f_putp(&txt0, &Fil);

				//Punkty - Markers
				f_putp(&txt23, &Fil); f_putp(&txt13, &Fil); f_putp(&txt0, &Fil);
				write_point("P1", &rcfa.P1, refhb, altmode | GE_EXTRUDE);
				write_point("P2", &rcfa.P2, refhb, GE_NOT_VISIBLE | altmode | GE_EXTRUDE);
				write_point("C", &rcfa.C, refhb, GE_NOT_VISIBLE | altmode | GE_EXTRUDE);
				write_point("A", &rcfa.A, refhb, GE_NOT_VISIBLE | altmode | GE_EXTRUDE);
				write_point("B", &rcfa.B, refhb, GE_NOT_VISIBLE | altmode | GE_EXTRUDE);

				//Flight plane
				f_putp(&txt6, &Fil); f_putp(&txt23, &Fil); f_putp(&txt8, &Fil); f_putp(&txt9, &Fil); f_putp(&txt0, &Fil);
				write_line("Dist", &rcfa.P1, refhb, &rcfa.C, refhb, GE_STYLE_PL | altmode);
				write_line("Flight line", &rcfa.A, refhb, &rcfa.B, refhb, GE_STYLE_PL | GE_NOT_VISIBLE | altmode);
				if (ini.floor<2) {
					write_line("Top", &rcfa.A, refht, &rcfa.B, refht, GE_STYLE_PL | GE_NOT_VISIBLE | GE_ALT_ABSOLUTE);
					write_line("A", &rcfa.A, refhb, &rcfa.A, refht, GE_STYLE_PL | GE_NOT_VISIBLE | GE_ALT_ABSOLUTE | GE_EXTRUDE);
					write_line("B", &rcfa.B, refhb, &rcfa.B, refht, GE_STYLE_PL | GE_NOT_VISIBLE | GE_ALT_ABSOLUTE | GE_EXTRUDE);
				}

				f_sync(&Fil);

				//Dodatkowe skoœne linie
				write_line("P1A", &rcfa.P1, refhb, &rcfa.A, refhb, GE_STYLE_PL | altmode | GE_NOT_VISIBLE);
				write_line("P1B", &rcfa.P1, refhb, &rcfa.B, refhb, GE_STYLE_PL | altmode | GE_NOT_VISIBLE);

				//Szerokoœæ linni lotu - Area Center
				f_putp(&txt6, &Fil); f_putp(&txt23, &Fil); f_putp(&txt10, &Fil); f_putp(&txt9, &Fil); f_putp(&txt0, &Fil);
				write_line("fl1", &rcfa.SAn, refhb, &rcfa.SBn, refhb, GE_STYLE_LW | altmode);
				write_line("fl2", &rcfa.SAf, refhb, &rcfa.SBf, refhb, GE_STYLE_LW | altmode);
				if (ini.floor<2) {
					write_line("fl3", &rcfa.SAn, refht, &rcfa.SBn, refht, GE_STYLE_LW | GE_ALT_ABSOLUTE);
					write_line("fl4", &rcfa.SAf, refht, &rcfa.SBf, refht, GE_STYLE_LW | GE_ALT_ABSOLUTE);
					write_line("fl5", &rcfa.SAn, refhb, &rcfa.SAn, refht, GE_STYLE_LW | GE_ALT_ABSOLUTE | GE_EXTRUDE);
					write_line("fl6", &rcfa.SAf, refhb, &rcfa.SAf, refht, GE_STYLE_LW | GE_ALT_ABSOLUTE | GE_EXTRUDE);
					write_line("fl7", &rcfa.SBn, refhb, &rcfa.SBn, refht, GE_STYLE_LW | GE_ALT_ABSOLUTE | GE_EXTRUDE);
					write_line("fl8", &rcfa.SBf, refhb, &rcfa.SBf, refht, GE_STYLE_LW | GE_ALT_ABSOLUTE | GE_EXTRUDE);
				}

				//Margins
				f_putp(&txt6, &Fil); f_putp(&txt23, &Fil); f_putp(&txt11, &Fil); f_putp(&txt9, &Fil); f_putp(&txt0, &Fil);
				write_line("fb1", &rcfa.AAn, refhb, &rcfa.ABn, refhb, GE_STYLE_AW | GE_NOT_VISIBLE | altmode);
				write_line("fb2", &rcfa.AAf, refhb, &rcfa.ABf, refhb, GE_STYLE_AW | GE_NOT_VISIBLE | altmode);
				if (ini.floor<2) {
					write_line("fb3", &rcfa.AAn, refht, &rcfa.ABn, refht, GE_STYLE_AW | GE_ALT_ABSOLUTE | GE_NOT_VISIBLE);
					write_line("fb4", &rcfa.AAf, refht, &rcfa.ABf, refht, GE_STYLE_AW | GE_ALT_ABSOLUTE | GE_NOT_VISIBLE);
					write_line("fb5", &rcfa.AAn, refhb, &rcfa.AAn, refht, GE_STYLE_AW | GE_ALT_ABSOLUTE | GE_NOT_VISIBLE | GE_EXTRUDE);
					write_line("fb6", &rcfa.AAf, refhb, &rcfa.AAf, refht, GE_STYLE_AW | GE_ALT_ABSOLUTE | GE_NOT_VISIBLE | GE_EXTRUDE);
					write_line("fb7", &rcfa.ABn, refhb, &rcfa.ABn, refht, GE_STYLE_AW | GE_ALT_ABSOLUTE | GE_NOT_VISIBLE | GE_EXTRUDE);
					write_line("fb8", &rcfa.ABf, refhb, &rcfa.ABf, refht, GE_STYLE_AW | GE_ALT_ABSOLUTE | GE_NOT_VISIBLE | GE_EXTRUDE);
				}

				//END Folder
				f_putp(&txt6, &Fil);
				f_putp(&txt6, &Fil);
				f_putp(&txt0, &Fil);
				f_sync(&Fil);
			}

			//Inicjalizacja œladu GPS
			f_putp(&txt23, &Fil); f_putp(&txt22, &Fil); f_putp(&txt0, &Fil);
			f_putp(&txt4, &Fil); f_putp(&txt0, &Fil);

			//Tryb wysokoœci dla œladu
			if (ini.floor<=2) { f_putp(&txt38, &Fil); f_putp(&txt0, &Fil); }
			//if (ini.floor2==2) { f_putp(&txt4, &Fil); f_putp(&txt0, &Fil); }

			//f_sync(&Fil);
			f_close(&Fil);

			//Reset punktacji
			logger.gztime = 0;
			logger.tstart = getGpsTimeSec();
			logger.maxspeed = 0.0;
			logger.maxalt = -9999.0;
			strcpy(logger.ctime, gpsData.ctime);

			//Inicjalizacja bufora logera
			logger.i = 0;

			//Loger w³¹czony
			logger.enabled = 1;

			sd_logp(1, &txt64);
		}
		else {
			sd_logp(1, &txt65);
		}
	}

	//Odnowienie czasu oczekiwania na GPS
	t_gpsBreak = logger.gpsBreak_time;

	//Logowanie aktywnych stelit
	gpsData.numsat_max = gpsData.numsat;
	gpsData.numsat_min = gpsData.numsat;
	gpsData.numsat_prev = 0;

	//W³¹czenie RX
	uart_enable_rx();
}

void logger_write_buf() {
	if ((sd_mount()==FR_OK) && (open_append(&Fil, (char *)logger.fname)==FR_OK)) {
		for (uint8_t i=0; i<logger.i; i++) {
			logger_write_coords(&logger.buf[i].w3d.w, logger.buf[i].w3d.alt, &logger.buf[i]);
		}
		f_close(&Fil);
	}

	//Oznaczenie bufora jako pustego niezale¿nie czy zapis siê powiód³
	logger.i = 0;
}

void logger_close() {
	char str[20];

	LED2_ON;
	t_led2=BLINK_TIME;

	//Wy³¹czenie RX
	uart_disable_rx();

	//Zapis pozosta³ych danych w buforze
	if (logger.i>0)	logger_write_buf();

	if (sd_mount()==FR_OK) {
		if (open_append(&Fil, (char *)logger.fname)==FR_OK) {
			//Zamkniêcie œladu GPS
			f_putp(&txt14, &Fil);
			//f_sync(&Fil);

			if ((rcfa.status & SET_RTB)==SET_RTB) {
				//Flight time - przekroczenie pó³nocy
				uint32_t flighttime = getGpsTimeSec();
				if (flighttime<logger.tstart) flighttime+=86400;	//Dodanie 24h
				flighttime = flighttime-logger.tstart;

				double tzone = (double)logger.gztime/1000.0*(double)CHKPOS_DELAY;	//Czas w centrum
				double score = tzone/(double)flighttime*100.0*1000.0;				//Czas w centrum jako % czasu lotu

				//Description
				f_putp(&txt27, &Fil); f_putp(&txt0, &Fil); f_putp(&txt0, &Fil);
				f_puts("Date: ", &Fil);																		f_puts(format_date(gpsData.cdate), &Fil);
				f_puts("\r\nStart time: ", &Fil);															f_puts(format_time(logger.ctime, TIME_TZONE), &Fil);
				f_puts("\r\nEnd time: ", &Fil);																f_puts(format_time(gpsData.ctime, TIME_TZONE), &Fil);
				f_puts("\r\n\r\nRating", &Fil);
				f_puts("\r\nMax speed: ", &Fil);			dtostrf(knots2kph(logger.maxspeed),1,2,str);	f_puts(str, &Fil);
				f_puts(" kph\r\nMax altitude: ", &Fil);		dtostrf((logger.maxalt-rcfa.refalt),1,2,str);	f_puts(str, &Fil);
				f_puts(" m r.t.g.\r\nFlight time: ", &Fil);													f_puts(format_ft(flighttime), &Fil);
				f_puts("\r\nGreen Zone time: ", &Fil);														f_puts(format_ft((uint16_t)tzone), &Fil);
				f_puts("\r\nScore: ", &Fil);				dtostrf(score,1,0,str);							f_puts(str, &Fil);
				f_putp(&txt37, &Fil);
				f_sync(&Fil);
			}

			//Zakoñczenie œladu GPS
			f_putp(&txt7, &Fil);
			f_putp(&txt6, &Fil);
			f_putp(&txt0, &Fil);

			//Zakoñczenie pliku KML
			f_putp(&txt21, &Fil);

			//Zamkniêcie pliku
			f_close(&Fil);
		}
		else {
			sd_logp(1, &txt69);
		}

		//Minimalna i makymalna liczba satelit
		sd_log_val(1, "Sat MIN", 0, gpsData.numsat_min);
		sd_log_val(1, "Sat MAX", 0, gpsData.numsat_max);
	}

	//W³¹czenie RX
	uart_enable_rx();

	t_gpsBreak = logger.gpsBreak_time;

	//Logger wy³¹czony
	logger.enabled = 0;
}

DWORD get_fattime() {
/*	Currnet local time is returned with packed into a DWORD value. The bit field is as follows:

	define GET_FATTIME()	((DWORD)(_NORTC_YEAR - 1980) << 25 | (DWORD)_NORTC_MON << 21 | (DWORD)_NORTC_MDAY << 16)

	bit31:25
	    Year origin from the 1980 (0..127)
	bit24:21
	    Month (1..12)
	bit20:16
	    Day of the month(1..31)
	bit15:11
	    Hour (0..23)
	bit10:5
	    Minute (0..59)
	bit4:0
	    Second / 2 (0..29)
*/
	//gpsData.date | ddmmyy
	uint8_t dd = (gpsData.cdate[0]-48)*10 + (gpsData.cdate[1]-48);
	uint8_t dm = (gpsData.cdate[2]-48)*10 + (gpsData.cdate[3]-48);
	uint16_t dy = 2000+((gpsData.cdate[4]-48)*10 + (gpsData.cdate[5]-48));

	//gpsData.time | hhmmss
	int8_t th = (gpsData.ctime[0]-48)*10 + (gpsData.ctime[1]-48);
	uint8_t tm = (gpsData.ctime[2]-48)*10 + (gpsData.ctime[3]-48);
	uint8_t ts = ((gpsData.ctime[4]-48)*10 + (gpsData.ctime[5]-48))/2;

	//Uwzglêdnienie strefy czasowej
	th += ini.tzone;
	if (th>23) th-=24;
	else if (th<0) th+=24;

	return ((DWORD)(dy-1980)<<25 | (DWORD)dm<<21 | (DWORD)dd<<16) | (DWORD)th<<11 | (DWORD)tm<<5 | (DWORD)ts;
}

#endif
