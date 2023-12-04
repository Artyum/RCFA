/*
 * tx.c
 *
 *  Created on: 8 mar 2015
 *      Author: Dexter
 */

#include "../config.h"
#if RADIO_MODE==1

#include <string.h>
#include "radio.h"
#include "radio_tx.h"
#include "../SPI/spi.h"
#include "../UART/uart.h"
#include "../GPS/gps.h"
#include "../RCFA/rcfa.h"
#include "../SD/sd.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

volatile uint16_t t_radio_tx;
uint8_t txtab[12];

void radio_hop_tx() {
	radio_mode_rx();

	//Ustalenie nastêpnego kana³u
	//int8_t sub = fhss.ch + num;
	//if (sub>=FHSS_NUM) sub = sub-FHSS_NUM;
	//if (sub<0) sub = FHSS_NUM+sub;
	//fhss.ch = sub;

	if (++fhss.ch>=FHSS_NUM) fhss.ch = 0;
	radio_set_freq(get_freq(fhss.list[fhss.ch]));
}

void radio_init_tx() {
	spi_disable();
	spi_enable(SPI_MODE_4);

	//Sprawdzenie czy jest po³¹czenie z radiem
	hw.radio = radio_detect();

	if (ini.radio==0) {
		//Ustawienie trybu uœpienia
		if (hw.radio) radio_mode_sleep();
		radio.enabled = 0;
	}
	else {
		radio.enabled = hw.radio;
	}

	if (radio.enabled) {
		//Wspólne ustawienia dla Rx i Tx
		radio_init_common();

		#if RADIO_20DBM==0
		//RegPaLevel
		//radio_cmd(REG_PALEVEL,0b10011111);	//PA0 On | Power -18+31 -> +13dBm
		radio_cmd(REG_PALEVEL,0b10000000);		//PA0 On | Power -18+0 -> -18dBm

		//Over Current Protection
		radio_cmd(REG_OCP,0b00011010);	//OCP enabled | 95mA

		//Test Registers
		radio_cmd(REG_TESTLNA,0x1B);	//Sensitivity boost - Normal mode
		radio_cmd(REG_TESTPA1,0x55);	//Normal mode RX or PA0
		radio_cmd(REG_TESTPA2,0x70);	//Normal mode RX or PA0
		radio_cmd(REG_TESTDAGC,0x30);	//Dagc On -
		//radio_cmd(REG_TESTDAGC,0x20);	//Dagc On
		#else

		//RegPaLevel
		//Pocz¹tkowa moc nadawania
		radio_set_power(POWER_INIT);

		//Over Current Protection
		radio_cmd(REG_OCP,0b00001111);	//OCP disabled

		//Test Registers
		radio_cmd(REG_TESTLNA,0x1B);	//Sensitivity boost - Normal mode
		radio_cmd(REG_TESTPA1,0x5D);	//+20dBm
		radio_cmd(REG_TESTPA2,0x7C);	//+20dBm
		radio_cmd(REG_TESTDAGC,0x30);	//Dagc On -
		//radio_cmd(REG_TESTDAGC,0x20);	//Dagc On
		#endif

		//Przerwania DioMapping (default)
		//radio_cmd(REG_DIOMAPPING1,0x00);			//Dio 0-3: 00
		//radio_cmd(REG_DIOMAPPING2,0b00000111);	//Dio 4-5: 00 | CLKOUT Off

		//RegLna
		//radio_cmd(REG_LNA,0x00);	//LNA 50ohms | gain set by internal AGC

		//Na pocz¹tku nic nie jest wysy³ane
		radio.status = 0;

		//Wejœcie w tryb Standby
		radio_mode_standby();
	}
	spi_disable();
}

uint8_t radio_tx_payload() {
	if (radio.enabled==0) return 0;

	//Jeœli w Fifo s¹ dane do wys³ania
	if (radio.status==1) {
		spi_enable(SPI_MODE_4);
		radio_mode_tx();		//Wys³anie danych
		spi_disable();
		radio.status = 2;
	}

	//Jeœli dane wys³ane
	else if ((radio.status==2) && (INT_PACKET_SENT)) {
		spi_enable(SPI_MODE_4);
		radio_hop_tx();
		radio_mode_standby();	//Wyczyszczenie Fifo
		spi_disable();
		radio.status = 0;
	}

	return radio.status;
}

//1 bajt - Numsat
void radio_tx_numsat(uint8_t b) {
	if ((radio.status!=0)||(radio.enabled==0)) return;
	radio.status=1;

	txtab[0] = 1;
	txtab[1] = b;

	spi_enable(SPI_MODE_4);
	spi_write_burst(REG_FIFO, txtab, 2);
	spi_disable();
}

//2 bajty - Type (1 bit) + Alarm|VarioAlarm (15 bitow)
void radio_tx_type_val(uint8_t type, int16_t val) {
	if ((radio.status!=0)||(radio.enabled==0)) return;
	radio.status=1;

	//Przesuniêcie przedzia³u alarmu
	val+=ALARM_PUSH;

	/*
	Alarm 10000-30000 21458
	t1010011 11010010
	*/
	txtab[0]  = 2;
	txtab[1]  = (type & 0x1)<<7;
	txtab[1] |= (val & 0x7F00)>>8;
	txtab[2]  = (val & 0xFF);

	spi_enable(SPI_MODE_4);
	spi_write_burst(REG_FIFO, txtab, 3);
	spi_disable();
}

//3 bajty - txpos
void radio_tx_pos(uint8_t type, uint32_t val) {
	if ((radio.status!=0)||(radio.enabled==0)) return;
	radio.status=1;

	txtab[0]  = 3;
	txtab[1]  = (uint32_t)type << 6;
	txtab[1] |= (val & 0x3F0000)>>16UL;
	txtab[2]  = (val & 0xFF00)>>8UL;
	txtab[3]  = (val & 0xFF);

	spi_enable(SPI_MODE_4);
	spi_write_burst(REG_FIFO, txtab, 4);
	spi_disable();
}

//4 bajty - Konfiguracja | Informacje
void radio_tx_info(uint8_t type) {
	if ((radio.status!=0) || (radio.enabled==0)) return;
	radio.status=1;

	txtab[0] = 4;
	txtab[1] = type;
	txtab[2] = ini.vol;
	txtab[3] = ini.variomode;
	txtab[4] = 0; //Nieu¿ywane

	spi_enable(SPI_MODE_4);
	spi_write_burst(REG_FIFO, txtab, 5);
	spi_disable();
}

//10 bajtów - Pozycja referencyjna i data GPS
void radio_tx_refpos() {
	if ((radio.status!=0)||(radio.enabled==0)) return;
	radio.status=1;

	//Przygotowanie danych do wys³ania
	uint8_t d = (gpsData.cdate[0]-48)*10 + (gpsData.cdate[1]-48);
	uint8_t m = (gpsData.cdate[2]-48)*10 + (gpsData.cdate[3]-48);
	uint8_t y = (gpsData.cdate[4]-48)*10 + (gpsData.cdate[5]-48);
	uint32_t lat = (rcfa.R.lat+(double)LAT_PUSH)*(double)ACC6;		//28 bitów	0 - 179999999
	uint32_t lon = (rcfa.R.lon+(double)LON_PUSH)*(double)ACC6;		//29 bitów	0 - 359999999
	//uint32_t lat = (rcfa.X.lat*ACC8)+(LAT_PUSH*ACC8);
	//uint32_t lon = (rcfa.X.lon*ACC8)+(LON_PUSH*ACC8);

	txtab[0] = REFPOS_TAB_CNT-1;

	//cdate 16b
	txtab[1]  = (d & 0x1F);
	txtab[1] |= ((m & 0x7)<<5);
	txtab[2]  = ((m & 0x8)>>3);
	txtab[2] |= (y<<1);

	//lat 28b
	txtab[3] = (lat & 0xFF);
	txtab[4] = ((lat & 0xFF00)>>8);
	txtab[5] = ((lat & 0xFF0000)>>16UL);
	txtab[6] = ((lat & 0xF000000)>>24UL);

	//lon 29b
	txtab[7] = (lon & 0xFF);
	txtab[8] = ((lon & 0xFF00)>>8);
	txtab[9] = ((lon & 0xFF0000)>>16UL);
	txtab[10]= ((lon & 0x1F000000)>>24UL);

	//Wysy³anie
	spi_enable(SPI_MODE_4);
	spi_write_burst(REG_FIFO, txtab, REFPOS_TAB_CNT);
	spi_disable();
}

#endif
