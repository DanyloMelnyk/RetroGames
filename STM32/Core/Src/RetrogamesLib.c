/*
 * RetrogamesLib.cpp - Library for work with Joystics and other hardwares.
 *
 *  Created on: 01.11.2019
 *  Ported to STM32 on: 26.01.2020
 *  Author: Danylo Melnyk (https://github.com/DanyloMelnyk)
 *  Created as part of RGC project (https://github.com/DanyloMelnyk/RetroGames)
 */

#include <max7219.h>
#include <RetrogamesLib.h>

#ifndef RETROGAMELIB2_H_
#define RETROGAMELIB2_H_
const bool icon[3][8][8] = {
{
		//Snake1,
		{0, 1, 1, 0, 0, 0, 0, 0},
		{1, 0, 0, 1, 0, 0, 0, 0},
		{1, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 0, 0, 0, 1},
		{1, 0, 0, 1, 0, 0, 1, 1},
		{0, 1, 1, 0, 0, 0, 0, 1},
		{0, 0, 0, 0, 0, 0, 0, 1}
}, {
		//Snake2
		{0, 0, 0, 0, 1, 1, 0, 1},
		{0, 0, 0, 1, 0, 0, 1, 1},
		{0, 0, 0, 0, 1, 0, 0, 1},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 0, 0, 1, 1, 0, 0},
		{1, 0, 0, 1, 0, 0, 1, 0},
		{1, 0, 0, 1, 0, 0, 1, 0},
		{0, 1, 1, 0, 0, 1, 0, 0}
}, {
		//Pong2
		{0, 1, 1, 1, 1, 1, 1, 1},
		{0, 0, 0, 0, 1, 0, 0, 1},
		{0, 0, 0, 0, 1, 0, 0, 1},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{1, 0, 0, 1, 0, 0, 0, 0},
		{1, 1, 0, 0, 1, 0, 0, 0},
		{1, 0, 1, 1, 0, 0, 0, 0}
} };

const bool digits[10][8][8] = { { { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 1, 1, 1,
		1, 0, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 1, 1, 0, 1, 1, 1, 0 }, { 0,
		1, 1, 1, 0, 1, 1, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 1, 1, 0, 0, 1,
		1, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0 } }, { { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0,
		0, 0, 1, 1, 0, 0, 0 }, { 0, 0, 0, 1, 1, 0, 0, 0 }, { 0, 0, 1, 1, 1, 0,
		0, 0 }, { 0, 0, 0, 1, 1, 0, 0, 0 }, { 0, 0, 0, 1, 1, 0, 0, 0 }, { 0, 0,
		0, 1, 1, 0, 0, 0 }, { 0, 1, 1, 1, 1, 1, 1, 0 } }, { { 0, 0, 0, 0, 0, 0,
		0, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 0,
		0, 0, 0, 1, 1, 0 }, { 0, 0, 0, 0, 1, 1, 0, 0 },
		{ 0, 0, 1, 1, 0, 0, 0, 0 }, { 0, 1, 1, 0, 0, 0, 0, 0 }, { 0, 1, 1, 1, 1,
				1, 1, 0 } }, { { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 1, 1, 1, 1,
		0, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 0, 0, 0, 0, 1, 1, 0 }, { 0, 0,
		0, 1, 1, 1, 0, 0 }, { 0, 0, 0, 0, 0, 1, 1, 0 },
		{ 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0 } }, { { 0, 0, 0,
		0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 1, 1, 0, 0 }, { 0, 0, 0, 1, 1, 1, 0, 0 },
		{ 0, 0, 1, 0, 1, 1, 0, 0 }, { 0, 1, 0, 0, 1, 1, 0, 0 }, { 0, 1, 1, 1, 1,
				1, 1, 0 }, { 0, 0, 0, 0, 1, 1, 0, 0 },
		{ 0, 0, 0, 0, 1, 1, 0, 0 } }, { { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 1, 1,
		1, 1, 1, 1, 0 }, { 0, 1, 1, 0, 0, 0, 0, 0 }, { 0, 1, 1, 1, 1, 1, 0, 0 },
		{ 0, 0, 0, 0, 0, 1, 1, 0 }, { 0, 0, 0, 0, 0, 1, 1, 0 }, { 0, 1, 1, 0, 0,
				1, 1, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0 } }, { { 0, 0, 0, 0, 0, 0,
		0, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 1,
		1, 0, 0, 0, 0, 0 }, { 0, 1, 1, 1, 1, 1, 0, 0 },
		{ 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 0, 1, 1, 1,
				1, 0, 0 } }, { { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 1, 1, 1, 1, 1,
		1, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 0, 0, 0, 1, 1, 0, 0 }, { 0, 0,
		0, 0, 1, 1, 0, 0 }, { 0, 0, 0, 1, 1, 0, 0, 0 },
		{ 0, 0, 0, 1, 1, 0, 0, 0 }, { 0, 0, 0, 1, 1, 0, 0, 0 } }, { { 0, 0, 0,
		0, 0, 0, 0, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 },
		{ 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0 }, { 0, 1, 1, 0, 0,
				1, 1, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 },
		{ 0, 0, 1, 1, 1, 1, 0, 0 } }, { { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 1,
		1, 1, 1, 0, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 }, { 0, 1, 1, 0, 0, 1, 1, 0 },
		{ 0, 0, 1, 1, 1, 1, 1, 0 }, { 0, 0, 0, 0, 0, 1, 1, 0 }, { 0, 1, 1, 0, 0,
				1, 1, 0 }, { 0, 0, 1, 1, 1, 1, 0, 0 } } };

const bool winMSG[8][94] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1,
		0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
		0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0 }, { 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
		1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0 }, { 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
		1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0 }, { 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1,
		1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0 }, { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
		0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
		0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		0 },
		{ 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
				1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
				1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0,
				0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
				1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
				0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,
				0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,
				1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, };

const bool scoreMessage[8][55] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0,
		0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0,
		0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0,
		0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		0 } };
#endif

/*const int melody[] = {
 NOTE_E7, NOTE_E7, 0, NOTE_E7,
 0, NOTE_C7, NOTE_E7, 0,
 NOTE_G7, 0, 0, 0,
 NOTE_G6, 0, 0, 0,

 NOTE_C7, 0, 0, NOTE_G6,
 0, 0, NOTE_E6, 0,
 0, NOTE_A6, 0, NOTE_B6,
 0, NOTE_AS6, NOTE_A6, 0,

 NOTE_G6, NOTE_E7, NOTE_G7,
 NOTE_A7, 0, NOTE_F7, NOTE_G7,
 0, NOTE_E7, 0, NOTE_C7,
 NOTE_D7, NOTE_B6, 0, 0,

 NOTE_C7, 0, 0, NOTE_G6,
 0, 0, NOTE_E6, 0,
 0, NOTE_A6, 0, NOTE_B6,
 0, NOTE_AS6, NOTE_A6, 0,

 NOTE_G6, NOTE_E7, NOTE_G7,
 NOTE_A7, 0, NOTE_F7, NOTE_G7,
 0, NOTE_E7, 0, NOTE_C7,
 NOTE_D7, NOTE_B6, 0, 0
 };
 //Mario main them tempo
 const int tempo[] = {
 12, 12, 12, 12,
 12, 12, 12, 12,
 12, 12, 12, 12,
 12, 12, 12, 12,

 12, 12, 12, 12,
 12, 12, 12, 12,
 12, 12, 12, 12,
 12, 12, 12, 12,

 9, 9, 9,
 12, 12, 12, 12,
 12, 12, 12, 12,
 12, 12, 12, 12,

 12, 12, 12, 12,
 12, 12, 12, 12,
 12, 12, 12, 12,
 12, 12, 12, 12,

 9, 9, 9,
 12, 12, 12, 12,
 12, 12, 12, 12,
 12, 12, 12, 12,
 };

 void HardwareController::gameOverBuzz()
 {
 const int underworld_melody[] = {
 NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
 NOTE_AS3, NOTE_AS4, 0,
 0,
 NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
 NOTE_AS3, NOTE_AS4, 0,
 0,
 NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
 NOTE_DS3, NOTE_DS4, 0,
 0,
 NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
 NOTE_DS3, NOTE_DS4, 0,
 0, NOTE_DS4, NOTE_CS4, NOTE_D4,
 NOTE_CS4, NOTE_DS4,
 NOTE_DS4, NOTE_GS3,
 NOTE_G3, NOTE_CS4,
 NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
 NOTE_GS4, NOTE_DS4, NOTE_B3,
 NOTE_AS3, NOTE_A3, NOTE_GS3,
 0, 0, 0
 };

 int size = sizeof(underworld_melody) / sizeof(int);
 //Underwolrd tempo
 const int underworld_tempo[] = {
 12, 12, 12, 12,
 12, 12, 6,
 3,
 12, 12, 12, 12,
 12, 12, 6,
 3,
 12, 12, 12, 12,
 12, 12, 6,
 3,
 12, 12, 12, 12,
 12, 12, 6,
 6, 18, 18, 18,
 6, 6,
 6, 6,
 6, 6,
 18, 18, 18, 18, 18, 18,
 10, 10, 10,
 10, 10, 10,
 3, 3, 3
 };

 for (int thisNote = 50; thisNote < size; thisNote++)
 {
 int noteDuration = 1000 / underworld_tempo[thisNote];

 tone(8, underworld_melody[thisNote], noteDuration);

 int pauseBetweenNotes = noteDuration * 1.30;
 delay(pauseBetweenNotes);

 // stop the tone playing:
 tone(8, 0, noteDuration);
 }
 }*/

void draw_menu() {
	clearAllDisplay();

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			setLed(0, i, j, icon[0][i][j]);
			setLed(1, i, j, icon[1][i][j]);
			//setLed(2, i, j, digits[0][i][j]);
			setLed(3, i, j, icon[2][i][j]);
			//setLed(4, i, j, digits[4][i][j]);
		}

		sendAll();
	}
}

Point joystickHome1 = { 0, 0 };
Point joystickHome2 = { 0, 0 };

const int joystickThreshold = 300;

void calibrate() {
	Point values = { 0, 0 };

	for (int i = 0; i < 10; i++) {
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		values.row += HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);

		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2, 100);
		values.col += HAL_ADC_GetValue(&hadc2);
		HAL_ADC_Stop(&hadc2);
	}
#ifdef SERIAL_DEBUG
		// TODO SERIAL DEBUG
#endif

	joystickHome1.row = values.row / 10;
	joystickHome1.col = values.col / 10;

	values.row = 0;
	values.col = 0;

	for (int i = 0; i < 10; i++) {
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		values.row += HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);

		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2, 100);
		values.col += HAL_ADC_GetValue(&hadc2);
		HAL_ADC_Stop(&hadc2);
	}

	joystickHome2.row = values.row / 10;
	joystickHome2.col = values.col / 10;
}

void waitJoystic() // common
{
	while (scanJoystick(1) == -1 && scanJoystick(2) == -1) {
		delay(1);
	}
}

void print_score(int score) //вивести рахунок (для змійки)
		{
	if (score < 0 || score > 999)
		return;

	uint8_t third = (uint8_t)(score % 10);
	uint8_t second = (uint8_t)((score / 10) % 10);
	uint8_t first = (uint8_t)((score / 100) % 10);

	char num_size;
	if (score > 99)
		num_size = 3;
	else if (score > 9)
		num_size = 2;
	else
		num_size = 1;

	for (int k = 0; k < (int) sizeof(scoreMessage[0]) + 4 + (num_size * 8); k++) //  + num_size * 8
			{
		for (int8_t i = 0; i < 8; i++) {
			for (int8_t j = 0; j < 24; j++) {
				if (j + k < 8)
					setLEDM(i + 8, j, false, false); // Пропуск 8 пікселів
				else if (j + k < (int) sizeof(scoreMessage[0]) + 8)
					setLEDM(i + 8, j, scoreMessage[i][j + k - 8], false); // Текст повідомлення
				else if (j + k < 63 + 8 * num_size) {
					if (j + k > (int) sizeof(scoreMessage[0]) + 8 * (num_size))
						setLEDM(i + 8, j,
								digits[third][i][j + k - 63 - 8 * (num_size - 1)],
								false);
					// Остання цифра рахунку
					else if (j + k
							> (int) sizeof(scoreMessage[0])
									+ 8 * (num_size - 1))
						setLEDM(i + 8, j,
								digits[second][i][j + k - 63
										- 8 * (num_size - 2)], false);
					else
						setLEDM(i + 8, j,
								digits[first][i][j + k - 63 - 8 * (num_size - 3)],
								false);
				} else
					setLEDM(i + 8, j, false, false);
			}
		}

		sendAll();
		if (k > 10)
			if (scanJoystick(1) != -1 || scanJoystick(2) != -1) {
				clearAllDisplay();
				delay(10);

				return;
			}

		delay(40);
	}

	clearAllDisplay();
	waitJoystic();
}

void win_msg(uint8_t player) // вивести повідомлення про перемогу
		{
	for (int k = 0; k < 98; k++) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 24; j++)
				if (j + k < 8)
					setLEDM(i + 8, j, false, false);
				else if (j + k < 16)
					setLEDM(i + 8, j, digits[player][i][j + k - 8], false);
				else if (j + k < 110)
					setLEDM(i + 8, j, winMSG[i][j + k - 16], false);
				else
					setLEDM(i + 8, j, false, false);
		}

		sendAll();
		delay(50);
	}

	clearAllDisplay();

	waitJoystic();
}

void choose_menu_item(char m, char choose) {
	for (int row = 0; row < 8; row++) // invert
			{
		for (int col = 0; col < 8; col++) {
			setLed(m, row, col, !(icon[choose - 1][row][col]));
		}

		send(m);
	}

	delay(10);

	for (int row = 0; row < 8; row++) // invert it back
			{
		for (int col = 0; col < 8; col++) {
			setLed(m, row, col, (icon[choose - 1][row][col]));
		}

		send(m);
	}

	delay(100);
}

void pong_score(int player1Score, int player2Score) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			setLed(1, 7 - i, j, digits[player2Score][j][i]);
			setLed(3, i, 7 - j, digits[player1Score][j][i]);
		}
		sendAll();
	}
}

int8_t scanJoystick(int8_t player) {
		int X, Y;

		if (player == 1) {
			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, 100);
			X = HAL_ADC_GetValue(&hadc1);
			HAL_ADC_Stop(&hadc1);

			HAL_ADC_Start(&hadc2);
			HAL_ADC_PollForConversion(&hadc2, 100);
			Y = HAL_ADC_GetValue(&hadc2);
			HAL_ADC_Stop(&hadc2);

			if (X < joystickHome1.row - joystickThreshold)
				return left;
			if (X > joystickHome1.row + joystickThreshold)
				return right;
			if (Y < joystickHome1.col - joystickThreshold)
				return down;
			if (Y > joystickHome1.col + joystickThreshold)
				return up;
		} else {
			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, 100);
			X = HAL_ADC_GetValue(&hadc1);
			HAL_ADC_Stop(&hadc1);

			HAL_ADC_Start(&hadc2);
			HAL_ADC_PollForConversion(&hadc2, 100);
			Y = HAL_ADC_GetValue(&hadc2);
			HAL_ADC_Stop(&hadc2);

#ifdef SERIAL_DEBUG
			// TODO SERIAL DEBUG
#endif

			if (X < joystickHome2.row - joystickThreshold)
				return right;
			if (X > joystickHome2.row + joystickThreshold)
				return left;
			if (Y < joystickHome2.col - joystickThreshold)
				return up;
			if (Y > joystickHome2.col + joystickThreshold)
				return down;
		}

		return -1;
}
