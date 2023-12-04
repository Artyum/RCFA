/*
 * radio.h
 *
 *  Created on: 4 mar 2015
 *      Author: awitczak
 */

#ifndef _RADIO_H_
#define _RADIO_H_

#include <avr/io.h>
#include "../config.h"

#if PCB_REVISION==0
#include "../CPU/cpu_1.0.h"
#elif PCB_REVISION==1
#include "../CPU/cpu_1.1.h"
#endif

//Czêstotliwoœæ wysy³ania danych przez radio
#define RADIO_TX			20

//Maksymalny czas oczekiwania przez odbiornik na dane z nadajnika. Po tym czasie uznaje siê brak kontaktu [ms]
#define RX_MAX_WAIT			30

//Ile razy pod rz¹d s¹ wysy³ane punknty Z,P i konfiguracja
#define RESEND_CNT			90
//#define RESEND_DELAY		20

//Rodzaj przekazywanej wiadomoœci
#define INFO_CONFIG			10
#define INFO_RTB			20		//Ready To Block
#define INFO_BLK			30
#define INFO_P1				40
#define INFO_P2				50
#define INFO_REFALT_SET		60

//Moc nadawania
#if RADIO_TEST_POWER==0
#define POWER_INIT			3
#define POWER_SEND_CONFIG	3
#define POWER_SEND_P1		3
#define POWER_SEND_P2		3
#define POWER_SEND_REFALT	4
#define POWER_SEND_REFPOS	4
#define POWER_SEND_RTB		4
#define POWER_SEND_BLK		4
#define POWER_FULL			31
#else
//Do testów
#define POWER_INIT			2
#define POWER_SEND_CONFIG	2
#define POWER_SEND_P1		2
#define POWER_SEND_P2		2
#define POWER_SEND_REFALT	2
#define POWER_SEND_REFPOS	2
#define POWER_SEND_RTB		2
#define POWER_SEND_BLK		2
#define POWER_FULL			2
#endif

//Indeksy dla punktów
#define RADIO_Z				1
#define RADIO_P				2
#define RADIO_S				3
#define RADIO_A				4
#define RADIO_B				5

//Czêstotliwoœci
#if BASE_FREQ==0

//Pasmo
#define CH_FREQ_MIN			433.05
#define CH_FREQ_MAX			434.79
#define CH_NUM				35			/* Liczba kana³ów w paœmie */
#define FHSS_NUM			35			/* Liczba kana³ów FHSS */

#elif BASE_FREQ==1

#define CH_FREQ_MIN			864.0
#define CH_FREQ_MAX			869.0
#define CH_NUM				40
#define FHSS_NUM			40

#elif BASE_FREQ==2

#define CH_FREQ_MIN			915.0
#define CH_FREQ_MAX			927.0
#define CH_NUM				40
#define FHSS_NUM			40

#elif BASE_FREQ==3
// 459MHz UK
#define CH_FREQ_MIN			458.525
#define CH_FREQ_MAX			459.475
#define CH_NUM				20
#define FHSS_NUM			20

#endif

//Jumper 1-2
#define FHSS_B1				15
#define FHSS_H1				43

//Jumper 2-3
#define FHSS_B2				20
#define FHSS_H2				41

//Jumper OFF
#define FHSS_B3				25
#define FHSS_H3				47

//Przeskok pomiêdzy kana³ami w paœmie
#define CH_HOP				((double)(CH_FREQ_MAX-CH_FREQ_MIN)/(double)(CH_NUM-1))

#define IS_JUMER_12			(!READ_BIT(FQ_PINPORT, FQ_PIN_1))
#define IS_JUMER_23			(!READ_BIT(FQ_PINPORT, FQ_PIN_2))

//Oznaczenie wysy³anych danych w pakiecie 2 bajty (1 bit)
#define TX2_TYPE_ALARM		0
#define TX2_TYPE_VALARM		1

//Oznaczenie wysy³anych danych w pakiecie 3 bajty (2 bity)
#define TX3_TYPE_NMEA0		0
#define TX3_TYPE_NMEA1		1
#define TX3_TYPE_NMEA2		2

//Adresy rejestrów RFM69HW
#define REG_FIFO			0x00
#define REG_OPMODE			0x01
#define REG_DATAMODUL		0x02
#define REG_BITRATEMSB		0x03
#define REG_BITRATELSB		0x04
#define REG_FDEVMSB			0x05
#define REG_FDEVLSB			0x06
#define REG_FRFMSB			0x07
#define REG_FRFMID			0x08
#define REG_FRFLSB			0x09
#define REG_AFCCTRL			0x0B
#define REG_VERSION			0x10
#define REG_PALEVEL			0x11
#define REG_OCP				0x13
#define REG_LNA				0x18
#define REG_RXBW			0x19
#define REG_RSSIVALUE		0x24
#define REG_DIOMAPPING1		0x25
#define REG_DIOMAPPING2		0x26
#define REG_IRQFLAGS1		0x27
#define REG_IRQFLAGS2		0x28
#define REG_RSSITHRESH		0x29
#define REG_PREAMBLEMSB		0x2C
#define REG_PREAMBLELSB		0x2D
#define REG_SYNCCONFIG		0x2E
#define REG_SYNCVALUE1		0x2F
#define REG_SYNCVALUE2		0x30
#define REG_SYNCVALUE3		0x31
#define REG_SYNCVALUE4		0x32
#define REG_SYNCVALUE5		0x33
#define REG_SYNCVALUE6		0x34
#define REG_SYNCVALUE7		0x35
#define REG_SYNCVALUE8		0x36
#define REG_PACKETCONFIG1	0x37
#define REG_PAYLOADLENGTH	0x38
#define REG_AUTOMODES		0x3B
#define REG_FIFOTHRESH		0x3C
#define REG_PACKETCONFIG2	0x3D
#define REG_TESTLNA			0x58
#define REG_TESTPA1			0x5A
#define REG_TESTPA2			0x5C
#define REG_TESTDAGC		0x6F
#define REG_TESTAFC			0x71

typedef struct {
	//Globalny w³¹cznik radia (niezale¿ny od tego czy sprzêt zosta³ wykryty czy nie)
	// 0 - Radio wy³¹czone
	// 1 - Radio w³¹czone
	uint8_t enabled;

	//Znacznik statusu radia
	// 0 - Standby / oczekiwanie na dane; Fifo empty
	// 1 - Standby / Dane gotowe do wys³ania
	// 2 - Tryb TX / Rozpoczêcie nadawania
	uint8_t status;

	//Aktualna moc nadawania
	uint8_t power;
} s_radio;

typedef struct {
	uint8_t list[FHSS_NUM];
	uint8_t ch;
} s_fhss;

extern s_radio radio;
extern s_fhss fhss;

//Zapis rejestru ze sprawdzeniem czy wartoœæ zosta³a zapisana poprawnie
void radio_cmd(uint8_t adr, uint8_t val);

//Wspólne ustawienia inicjalizacyjne dla Rx i Tx
void radio_init_common();

//Zwraca 1 jeœli jest kontakt z radiem. Sprawdzane przez odczyt rejestru RegVersion
uint8_t radio_detect();

//Ustawienie czêstotliwoœci nadawania i odbierania w MHz
void radio_set_freq(double freq);

//Obliczenie czêœtotliwoœæ na podstawie numeru kana³u
double get_freq(uint8_t ch);

//Odczyt rejestrów
//uint8_t getLnaCurrentGain();
//uint8_t getRssiValue();

void radio_mode_fs();		//Frequency Synthesizer mode
void radio_mode_sleep();	//Tryb uœpienia
void radio_mode_standby();	//Wejœcie w tyb Standby
void radio_mode_rx();		//Wejœcie w tyb Rx
#if RADIO_MODE==1
void radio_mode_tx();		//Wejœcie w tyb Tx
#endif

//Moc nadawania
void radio_set_power(uint8_t p);

//Odczyt flag
//uint8_t rf_FifoFull();
//uint8_t rf_FifoNotEmpty();
uint8_t rf_ModeReady();
//uint8_t rf_RxReady();
//uint8_t rf_TxReady();
//uint8_t rf_ModeRxReady();
//uint8_t rf_ModeTxReady();
//uint8_t rf_PayloadReady();
//uint8_t rf_CrcOk();
//uint8_t rf_PacketSent();

#endif /* _RADIO_H_ */
