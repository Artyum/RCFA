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

#ifndef CONFIG_H_
#define CONFIG_H_

//Frequency
// 0 - 433 MHz
// 1 - 868 MHz
// 2 - 915 MHz (USA/Australia)
// 3 - 459 MHz (UK)
//#define BASE_FREQ				-1

#if BASE_FREQ==0
// 433 MHz
#endif
#if BASE_FREQ==1
// 868 MHz
#endif
#if BASE_FREQ==2
// 915 MHz
#endif
#if BASE_FREQ==3
// 459 MHz
#endif

#if defined (__AVR_ATmega168P__)
// ATmega168P
#endif
#if defined (__AVR_ATmega644P__) || defined (__AVR_ATmega644PA__)
// ATmega644P
#endif

#if F_CPU==11059200
// 11.0592Mhz
#endif
#if F_CPU==16000000
// 16Mhz
#endif

//////////////////////////////////////////////////////////////////////////////////

//PCB revision
// 0 - rev.1.0
// 1 - rev.1.1
#define PCB_REVISION			1

#if defined (__AVR_ATmega168P__)

//Firmware version
#define FIRMWARE_VER			"RCFA.RX.1.32"

//CPU
//	0 - Atmega 168P	(RX)
//	1 - Atmega 644P	(TX)
#define CPU						0

//Radio compilation switch
// 0 - Tryb RX - Receiver
// 1 - Tryb TX - Transmitter
#define RADIO_MODE				0

//Radio High Power
// 0 - Disabled or RX mode
// 1 - Enabled (only in TX mode)
#define RADIO_20DBM				1

//Function switch
#define FUNC_PLAY_TONE			0
#define FUNC_MAP				0
#define CHK_RX					0

//Signal switch
#define DISABLE_SND_NO_SIG		1
#endif

#if defined (__AVR_ATmega644P__) || defined (__AVR_ATmega644PA__)

#define FIRMWARE_VER			"RCFA.TX.1.32"
#define CPU						1
#define RADIO_MODE				1
#define RADIO_20DBM				1
#define FUNC_MAP				0

#define RADIO_TEST_POWER		0

#endif

#endif /* CONFIG_H_ */
