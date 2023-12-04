/*
 * sound.h
 *
 *  Created on: 11 wrz 2015
 *      Author: awitczak
 */

#ifndef SOUND_SOUND_H_
#define SOUND_SOUND_H_

#include "../config.h"
#if RADIO_MODE==0

typedef struct {
	uint8_t lp;

	//G³oœnoœæ 0->100
	uint8_t vol;

	//Znacznik czy wy³¹czaæ dŸwiêk po up³ywie zadanego czasu
	uint8_t off;

	//double intervals;
	//double gradsnd;

	//Rodzaj odebranych danych
	uint16_t type;
} s_sound;

//2*pi
#define PI_M2				6.283185307

//Przedzia³ czêstotliwoœci g³oœnika
#define HALF_TONE			1.059463094
#define T1_PRESCALER		1

//#define SAMPLE_RATE		44100UL
//#define SAMPLE_RATE		33075UL
#define SAMPLE_RATE			22050UL
//#define SAMPLE_RATE		11025UL

#define TOP_SAMPLE			(F_CPU/(T1_PRESCALER*(1+SAMPLE_RATE)))
#define STOP_SAMPLE			0xFFFF
#define FREQ_MIN			450

#define VOLUME_MIN			0
#define VOLUME_MAX			100

#define MAX_SAMPLES			(SAMPLE_RATE/FREQ_MIN+1)
#define FREQ_MAX			(SAMPLE_RATE/8)

#define INIT_PAUSE			30
#define END_PAUSE			250

#define PAUSE				0
#define TEMPO				30

//Wartoœci rytmiczne nut (czasy w ms)
#define N1K					(1.5*N1)
#define N1					(2*N2)
#define N2K					(1.5*N2)
#define N2					(2*N4)
#define N4K					(1.5*N4)
#define N4					(2*N8)
#define N8K					(1.5*N8)
#define N8					(2*N16)
#define N16K				(1.5*N16)
#define N16					(2*N32)
#define N32K				(1.5*N32)
#define N32					(2*N64)
#define N64K				(1.5*N64)
#define N64					(TEMPO)

//Czêstotliwoœci
/*#define FA0			27.500000
#define FAH0		29.135235
#define FH0			30.867706
#define FC1			32.703196
#define FCH1		34.647829
#define FD1			36.708096
#define FDH1		38.890873
#define FE1			41.203445
#define FF1			43.653529
#define FFH1		46.249303
#define FG1			48.999429
#define FGH1		51.913087
#define FA1			55.000000
#define FAH1		58.270470
#define FH1			61.735413
#define FC2			65.406391
#define FCH2		69.295658
#define FD2			73.416192
#define FDH2		77.781746
#define FE2			82.406889
#define FF2			87.307058
#define FFH2		92.498606
#define FG2			97.998859
#define FGH2		103.826174
#define FA2			110.000000
#define FAH2		116.540940
#define FH2			123.470825
#define FC3			130.812783
#define FCH3		138.591315
#define FD3			146.832384
#define FDH3		155.563492
#define FE3			164.813778
#define FF3			174.614116
#define FFH3		184.997211
#define FG3			195.997718
#define FGH3		207.652349
#define FA3			220.000000
#define FAH3		233.081881
#define FH3			246.941651
#define FC4			261.625565
#define FCH4		277.182631
#define FD4			293.664768
#define FDH4		311.126984
#define FE4			329.627557
#define FF4			349.228231
#define FFH4		369.994423
#define FG4			391.995436
#define FGH4		415.304698
#define FA4			440.000000*/
#define FAH4		466.163762
#define FH4			493.883301
#define FC5			523.251131
#define FCH5		554.365262
#define FD5			587.329536
#define FDH5		622.253967
#define FE5			659.255114
#define FF5			698.456463
#define FFH5		739.988845
#define FG5			783.990872
#define FGH5		830.609395
#define FA5			880.000000
#define FAH5		932.327523
#define FH5			987.766603
#define FC6			1046.502261
#define FCH6		1108.730524
#define FD6			1174.659072
#define FDH6		1244.507935
#define FE6			1318.510228
#define FF6			1396.912926
#define FFH6		1479.977691
#define FG6			1567.981744
#define FGH6		1661.218790
#define FA6			1760.000000
#define FAH6		1864.655046
#define FH6			1975.533205
#define FC7			2093.004522
#define FCH7		2217.461048
#define FD7			2349.318143
#define FDH7		2489.015870
#define FE7			2637.020455
#define FF7			2793.825851
#define FFH7		2959.955382
#define FG7			3135.963488
#define FGH7		3322.437581
#define FA7			3520.000000
#define FAH7		3729.310092
#define FH7			3951.066410
#define FC8			4186.009045

//Klawisze
#define KA0			1
#define KAH0		2
#define KH0			3
#define KC1			4
#define KCH1		5
#define KD1			6
#define KDH1		7
#define KE1			8
#define KF1			9
#define KFH1		10
#define KG1			11
#define KGH1		12
#define KA1			13
#define KAH1		14
#define KH1			15
#define KC2			16
#define KCH2		17
#define KD2			18
#define KDH2		19
#define KE2			20
#define KF2			21
#define KFH2		22
#define KG2			23
#define KGH2		24
#define KA2			25
#define KAH2		26
#define KH2			27
#define KC3			28
#define KCH3		29
#define KD3			30
#define KDH3		31
#define KE3			32
#define KF3			33
#define KFH3		34
#define KG3			35
#define KGH3		36
#define KA3			37
#define KAH3		38
#define KH3			39
#define KC4			40
#define KCH4		41
#define KD4			42
#define KDH4		43
#define KE4			44
#define KF4			45
#define KFH4		46
#define KG4			47
#define KGH4		48
#define KA4			49
#define KAH4		50
#define KH4			51
#define KC5			52
#define KCH5		53
#define KD5			54
#define KDH5		55
#define KE5			56
#define KF5			57
#define KFH5		58
#define KG5			59
#define KGH5		60
#define KA5			61
#define KAH5		62
#define KH5			63
#define KC6			64
#define KCH6		65
#define KD6			66
#define KDH6		67
#define KE6			68
#define KF6			69
#define KFH6		70
#define KG6			71
#define KGH6		72
#define KA6			73
#define KAH6		74
#define KH6			75
#define KC7			76
#define KCH7		77
#define KD7			78
#define KDH7		79
#define KE7			80
#define KF7			81
#define KFH7		82
#define KG7			83
#define KGH7		84
#define KA7			85
#define KAH7		86
#define KH7			87
#define KC8			88

void sound_init();
void sound_off();
void play_pause(uint16_t time);
void play_freq(double freq, uint16_t time, uint8_t vol);
//void play_key(uint8_t key, uint16_t time, double vol);
void chg_volume(int8_t c);

#if FUNC_PLAY_TONE==1
void play_key(uint8_t tone, uint16_t time, double vol);
void play_tone(uint8_t tone, uint16_t time, double vol);
#endif

extern s_sound snd;
extern volatile uint16_t t_buzzer;

#endif
#endif /* SOUND_SOUND_H_ */
