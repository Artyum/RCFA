/*
 * radio.c
 *
 *  Created on: 4 mar 2015
 *      Author: awitczak
 *
 *  Lista czêstotliwoœci
 *  http://pl.wikipedia.org/wiki/Pasmo_70_cm
 */

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "radio.h"
#include "../config.h"
#include "../RCFA/rcfa.h"
#include "../SPI/spi.h"
#include "../UART/uart.h"
#include "../SD/sd.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

s_radio radio;
s_fhss fhss;

void set_fhsslist(uint8_t begin, uint8_t hop) {
	fhss.list[0] = begin;
	for (uint8_t i=1; i<FHSS_NUM; i++) {
		fhss.list[i] = (fhss.list[i-1] + hop) % CH_NUM;
		//uart_putul(i,10); uart_puts("="); uart_putul(fhss.list[i],10); uart_puts("; "); uart_putd(get_freq(fhss.list[i]),3); uart_nl();
	}
	fhss.ch = 0;

	//Logowanie
	#if RADIO_MODE==1
		#if BASE_FREQ==0
		char info[] = "FHSS Enabled (433MHz)";
		#elif BASE_FREQ==1
		char info[] = "FHSS Enabled (868MHz)";
		#elif BASE_FREQ==2
		char info[] = "FHSS Enabled (915MHz)";
		#elif BASE_FREQ==3
		char info[] = "FHSS Enabled (459MHz)";
		#endif
		sd_log(1, info);
		sd_log_val(5, "FREQ MIN", CH_FREQ_MIN, 0);
		sd_log_val(5, "FREQ MAX", CH_FREQ_MAX, 0);
		sd_log_val(5, "CH NUM", 0, CH_NUM);
		sd_log_val(5, "CH HOP", CH_HOP, 0);
	#else
		uart_putsP(PSTR("FHSS enabled"),1);
		#if BASE_FREQ==0
		uart_putsP(PSTR("Freq 433"),1);
		#elif BASE_FREQ==1
		uart_putsP(PSTR("Freq 868"),1);
		#elif BASE_FREQ==2
		uart_putsP(PSTR("Freq 915"),1);
		#elif BASE_FREQ==3
		uart_putsP(PSTR("Freq 459 (UK)"),1);
		#endif
	#endif
}

double get_freq(uint8_t ch) {
	return (double)CH_FREQ_MIN+(double)ch*(double)CH_HOP;
}

void radio_cmd(uint8_t adr, uint8_t val) {
	uint8_t i = SPI_LOOP_CNT;

	//uart_puts("adr=0x"); uart_putl(adr,16); uart_puts("|val=0x"); uart_putl(val,16); uart_nl();

	//Próba zapisu do rejestu
	do {
		//uart_putc('+');
		spi_write(adr,val);
		_delay_us(10);
	} while ((spi_read(adr)!=val) && (--i));

	if (i==0) {
		#if RADIO_MODE==0
		uart_putsP(PSTR("Radio err.adr: 0x"),0); uart_putl(adr,16); uart_nl();
		#else
		sd_logp(1, &txt55);
		#endif
		system_stop();
	}
}

void radio_init_common() {
	spi_disable();
	//spi_enable(SPI_MODE_4);
	//spi_enable(SPI_MODE_16 | SPI_MODE_DR);
	spi_enable(SPI_MODE_16);

	if (IS_JUMER_12) {
		set_fhsslist(FHSS_B1,FHSS_H1);
		#if RADIO_MODE==1
		sd_logp(1, &txt60);
		#else
		uart_putsP(PSTR("Jumper 1-2"),1);
		#endif
	}
	else if (IS_JUMER_23) {
		set_fhsslist(FHSS_B2,FHSS_H2);
		#if RADIO_MODE==1
		sd_logp(1, &txt61);
		#else
		uart_putsP(PSTR("Jumper 2-3"),1);
		#endif
	}
	else { /*JUMER OFF*/
		set_fhsslist(FHSS_B3,FHSS_H3);
		#if RADIO_MODE==1
		sd_logp(1, &txt59);
		#else
		uart_putsP(PSTR("Jumper off"),1);
		#endif
	}

	//Czêstotliwoœæ pocz¹tkowa
	radio_set_freq(get_freq(fhss.list[fhss.ch]));
	radio_mode_fs();

	spi_disable();
	spi_enable(SPI_MODE_4);

	//Blokada pinów
	//BIT_LOW(FQ_PORT, FQ_PIN_1);
	//BIT_LOW(FQ_PORT, FQ_PIN_2);

	//Czyszczenie Fifo i wejœcie w tryb Sleep
	radio_mode_sleep();	while (!rf_ModeReady());
	radio_mode_rx();	while (!rf_ModeReady());
	radio_mode_sleep();	while (!rf_ModeReady());

	//RegDataModul
	radio_cmd(REG_DATAMODUL,0b00000000);	//Packet mode | FSK | no gaussian shaping
	//radio_cmd(REG_DATAMODUL,0b00000010);	//Packet mode | FSK | Gaussian shaping BT=0.5 -

	//RegBitrate
	radio_cmd(REG_BITRATEMSB,0x0D);		radio_cmd(REG_BITRATELSB,0x05);		//9.6 kbps
	//radio_cmd(REG_BITRATEMSB,0x1A);	radio_cmd(REG_BITRATELSB,0x0B); 	//4.8 kbps
	//radio_cmd(REG_BITRATEMSB,0x34);	radio_cmd(REG_BITRATELSB,0x15);		//2.4 kbps
	//radio_cmd(REGBITRATEMSB,0x68);	radio_cmd(REGBITRATELSB,0x2b);		//1.2 kbps

	//Frequency deviation
	//radio_cmd(REG_FDEVMSB,0x00); radio_cmd(REG_FDEVLSB,0x52);		//5KHz -
	radio_cmd(REG_FDEVMSB,0x03); radio_cmd(REG_FDEVLSB,0x33);		//50KHz

	//RegRxBw
	#if 1
		#if BASE_FREQ==0
		radio_cmd(REG_RXBW,0b01001011);			//FSK Channel filter bandwidth 50kHz
		#elif BASE_FREQ==1
		radio_cmd(REG_RXBW,0b01001010);			//FSK Channel filter bandwidth 100kHz
		#elif BASE_FREQ==2
		radio_cmd(REG_RXBW,0b01001010);			//FSK Channel filter bandwidth 100kHz
		#elif BASE_FREQ==3
		radio_cmd(REG_RXBW,0b01001011);			//FSK Channel filter bandwidth 50kHz
		#endif
	#else
		//radio_cmd(REG_RXBW,0b01010101);		//FSK Channel filter bandwidth 10.4kHz
		//radio_cmd(REG_RXBW,0b01001100);		//FSK Channel filter bandwidth 25kHz -
		//radio_cmd(REG_RXBW,0b01001011);		//FSK Channel filter bandwidth 50kHz
		radio_cmd(REG_RXBW,0b01001010);			//FSK Channel filter bandwidth 100kHz
		//radio_cmd(REG_RXBW,0b01000010);		//FSK Channel filter bandwidth 125kHz
	#endif

	//Fifo overrun - ustawienie flagi bez potwierdzenia zapisu
	//spi_write(REG_IRQFLAGS2,0x10);

	//RSSI trigger level
	//radio_cmd(REG_RSSITHRESH,0xE4);
	//radio_cmd(REG_RSSITHRESH,220);

	//SyncWord
	//radio_cmd(REG_SYNCCONFIG,0b10011000);		//Sync word On | FifoFillCondition SyncAddress interrupt occurs | SyncSize=3+1 -> 4 bajty | 0 errors
	radio_cmd(REG_SYNCCONFIG,0b10001000);		//Sync word On | FifoFillCondition SyncAddress interrupt occurs | SyncSize=1+1 -> 2 bajty | 0 errors
	//radio_cmd(REG_SYNCCONFIG,0b10010000);		//Sync word On | FifoFillCondition SyncAddress interrupt occurs | SyncSize=2+1 -> 3 bajty | 0 errors

	//SyncValue
	if (IS_JUMER_12) radio_cmd(REG_SYNCVALUE1,0xAA);		//10101010
	else if (IS_JUMER_23) radio_cmd(REG_SYNCVALUE1,0x32);	//00110010
	else radio_cmd(REG_SYNCVALUE1,0xCD);					//11001101

	radio_cmd(REG_SYNCVALUE2,0x55);
	radio_cmd(REG_SYNCVALUE3,0xAA);
	//radio_cmd(REG_SYNCVALUE4,0x55);
	//radio_cmd(REG_SYNCVALUE5,0x51);
	//radio_cmd(REG_SYNCVALUE6,0x03);

	//PacketConfig
	//radio_cmd(RegPacketConfig1,0b01010000);	//Fixed length | Whitening | CRC On | Clear FIFO and restart new packet reception | AddressFiltering Off
	//radio_cmd(REG_PACKETCONFIG1,0b10010000);	//Variable length | CRC On | Clear FIFO and restart new packet reception | AddressFiltering Off
	radio_cmd(REG_PACKETCONFIG1,0b11010000);	//Variable length | Whitening | CRC On | Clear FIFO and restart new packet reception | AddressFiltering Off

	//PayloadLength - D³ugoœæ pakietu w trybie Fixed lub maksymalna d³ugoœæ w trybie Variable [w bajtach]
	//radio_cmd(RegPayloadLength,9);			//1 Length byte + 8 bajtów danych
	radio_cmd(REG_PAYLOADLENGTH,20);
	//radio_cmd(REG_PAYLOADLENGTH,50); -
	//radio_cmd(REG_PAYLOADLENGTH,60);

	//TxStartCondition / wy³¹czone przy AutoMode?
	//radio_cmd(REG_FIFOTHRESH,0b10000001);		//TX when Fifo not empty | FifoThreshold = 1
	//radio_cmd(REG_FIFOTHRESH,0b00000001);		//TX on FifoLevel | FifoThreshold = 1
	//radio_cmd(REG_FIFOTHRESH,0b10001111);		//TX when Fifo not empty
	radio_cmd(REG_FIFOTHRESH,0b10000001);		//TX when Fifo not empty -

	//PacketConfig2
	//radio_cmd(REG_PACKETCONFIG2,0b00010010);

	//Liczba bajtów preambu³y
	radio_cmd(REG_PREAMBLEMSB,0x00); radio_cmd(REG_PREAMBLELSB,0x03);	//3 bajty default

	//LowBetaAfcOffset
	//radio_cmd(REG_TESTAFC,5);			//Offset = LowBetaAfcOffset x 488 Hz
}

/*uint8_t rf_FifoFull() {
	uint8_t reg = spi_read(REG_IRQFLAGS2);
	if ((reg & 0b10000000) == 0x80) return 1; else return 0;
}*/

/*uint8_t rf_FifoNotEmpty() {
	uint8_t reg = spi_read(REG_IRQFLAGS2);
	if ((reg & 0b01000000) == 0x40) return 1; else return 0;
}*/

uint8_t rf_ModeReady() {
	uint8_t reg = spi_read(REG_IRQFLAGS1);
	if ((reg & 0b10000000) == 0x80) return 1; else return 0;
}

/*uint8_t rf_RxReady() {
	uint8_t reg = spi_read(REG_IRQFLAGS1);
	if ((reg & 0b01000000) == 0x40) return 1; else return 0;
}*/

/*uint8_t rf_TxReady() {
	uint8_t reg = spi_read(REG_IRQFLAGS1);
	if ((reg & 0b00100000) == 0x20) return 1; else return 0;
}*/

/*uint8_t rf_ModeRxReady() {
	uint8_t reg = spi_read(REG_IRQFLAGS1);
	if ((reg & 0b11000000) == 0xC0) return 1; else return 0;
}*/

/*uint8_t rf_ModeTxReady() {
	uint8_t reg = spi_read(REG_IRQFLAGS1);
	if ((reg & 0b01010000) == 0xA0) return 1; else return 0;
}*/

/*uint8_t rf_PayloadReady() {
	uint8_t reg = spi_read(REG_IRQFLAGS2);
	//uart_putl(reg,16); uart_nl();
	if ((reg & 0b00000100) == 0x04) return 1; else return 0;

	//return INT_PR_STATUS;
}*/

/*uint8_t rf_CrcOk() {
	uint8_t reg = spi_read(REG_IRQFLAGS2);
	if ((reg & 0b00000010) == 0x02) return 1; else return 0;
}*/

/*uint8_t rf_PacketSent() {
	uint8_t reg = spi_read(REG_IRQFLAGS2);
	if ((reg & 0b00001000) == 0x08) return 1; else return 0;
}*/

/*uint8_t getLnaCurrentGain() {
	uint8_t reg = spi_read(REG_LNA);
	return ((reg>>3) & 7);
}

uint8_t getRssiValue() {
	uint8_t reg = spi_read(REG_RSSIVALUE);
	return -reg/2;
}*/

void radio_mode_sleep() {
	radio_cmd(REG_OPMODE,0x00);
	//while (!rf_ModeReady()); //Oczekiwanie na wejœcie w tryb
}

void radio_mode_fs() {
	radio_cmd(REG_OPMODE,0x08);
}

void radio_mode_standby() {
	//Wejœcie w tryb Standby
	//uint8_t reg = spi_read(REG_OPMODE);
	//reg = (reg & 0xe0); //Wyzerowanie bitów 4-2 'mode'
	//radio_cmd(REG_OPMODE,(reg|0x04)); //Ustawienie bitu nr 2 - Standby
	//while (!rf_ModeReady()); //Oczekiwanie na wejœcie w tryb

	radio_cmd(REG_OPMODE,0x04);
	//spi_write(REG_OPMODE,0x04);
}

void radio_mode_rx() {
	//Wejœcie w tryb Rx
	//uint8_t reg = spi_read(REG_OPMODE);
	//reg = (reg & 0xE0); //Wyzerowanie bitów 4-2 'mode'
	//radio_cmd(REG_OPMODE,(reg|0x10)); //Ustawienie bitu nr 4 - Rx
	//while (!rf_ModeReady()); //Oczekiwanie na wejœcie w tryb

	radio_cmd(REG_OPMODE,0x10);
	//spi_write(REG_OPMODE,0x10);
}

#if RADIO_MODE==1
void radio_mode_tx() {
	//Wejœcie w tryb Tx
	//uint8_t reg = spi_read(REG_OPMODE);
	//reg = (reg & 0xE0); //Wyzerowanie bitów 4-2 'mode'
	//radio_cmd(REG_OPMODE,(reg|0x0c)); //Ustawienie bitu nr 3,2 - Tx
	//while (!rf_ModeReady()); //Oczekiwanie na wejœcie w tryb

	radio_cmd(REG_OPMODE,0x0C);
	//spi_write(REG_OPMODE,0x0C);
}
#endif

void radio_set_freq(double freq) {
	//Carrier frequency F_RF = F_STEP * Frf(23,0)
	//433MHz	6C|40|00
	//434MHz	6C|80|00
	//434.52MHz	6C|A1|47
	//435MHz	6C|C0|00
	//868MHz	D9|00|00
	//869MHz	D9|40|00
	//870MHz	D9|80|00

	//uint8_t msb, mid, lsb;
	//uint8_t rmsb, rmid, rlsb;
	uint8_t f[3];
	uint8_t r[3];

	//Fstep = 61.0351563
	long int frf = (freq/61.0351563)*1000000.0;
	f[0] = (frf>>16) & 0xff;
	f[1] = (frf>>8) & 0xff;
	f[2] = frf & 0xff;

	//Wyœwietlenie hex czêstotliwoœci
	//uart_puts("Freq="); uart_putd(freq,3); uart_puts("|"); uart_putl(f[0],16); uart_puts("|"); uart_putl(f[1],16); uart_puts("|"); uart_putl(f[2],16); uart_nl();

	uint8_t cnt = SPI_LOOP_CNT; //Ogranicznik pêtli
	do {
		if (--cnt==0) {
			#if RADIO_MODE==1
			sd_logp(1, &txt53);
			#else
			uart_putsP(PSTR("Freq set err!"),1);
			#endif
			system_stop();
		}

		//elay_ms(1);
		spi_write_burst(REG_FRFMSB, f, 3);
		//delay_ms(1);
		_delay_ms(0.5);
		spi_read_burst(REG_FRFMSB, r, 3);

		//BIT_LOW(SPI_SS_PORT,SPI_SS);
		//delay_ms(1);
		//spi_tx(0x80|REG_FRFMSB);
		//spi_tx(msb);
		//spi_tx(mid);
		//spi_tx(lsb);
		//BIT_HIGH(SPI_SS_PORT,SPI_SS);

		//spi_write(REG_FRFMSB,msb);
		//spi_write(REG_FRFMID,mid);
		//spi_write(REG_FRFLSB,lsb);
		//delay_ms(10);

		//uart_putc('+');
		//rmsb = spi_read(REG_FRFMSB);
		//rmid = spi_read(REG_FRFMID);
		//rlsb = spi_read(REG_FRFLSB);

		//uart_puts("Ver="); uart_putl(spi_read(REG_VERSION),16);
		//uart_puts("Regs="); uart_putl(r[0],16); uart_puts("|"); uart_putl(r[1],16); uart_puts("|"); uart_putl(r[2],16); uart_nl();
	} while ((f[0]!=r[0]) || (f[1]!=r[1]) || (f[2]!=r[2]));
	//uart_putc('=');
}

uint8_t radio_detect() {
	uint8_t i = SPI_LOOP_CNT;

	//Próba po³¹czenia z radiem
	do {
		if (0x24 == spi_read(REG_VERSION)) return 1;
		//delay_ms(1);
		_delay_ms(0.5);
	} while (--i);

	return 0;
}

void radio_set_power(uint8_t p) {
	//Power -11dBm + p
	//radio_cmd(REG_PALEVEL,0b01100000);	//PA1 & PA2 | Power -11+0	-> -11dBm
	//radio_cmd(REG_PALEVEL,0b01101011);	//PA1 & PA2 | Power -11+11	-> 0dBm
	//radio_cmd(REG_PALEVEL,0b01111010);	//PA1 & PA2 | Power -11+26	-> +15dBm
	//radio_cmd(REG_PALEVEL,0b01111111);	//PA1 & PA2 | Power -11+31	-> +20dBm

	if (radio.power != p) {
		spi_enable(SPI_MODE_4);
		uint8_t power = (96 | p);
		radio_cmd(REG_PALEVEL,power);
		radio.power = p;
	}
}
