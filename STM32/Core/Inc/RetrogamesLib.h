/*
 * RetrogameLib.h - Library for work with Joystics and other hardwares.
 *
 *  Created on: 01.11.2019
 *  Ported to STM32 on: 26.01.2020
 *  Author: Danylo Melnyk (https://github.com/DanyloMelnyk)
 *  Created as part of RGC project (https://github.com/DanyloMelnyk/RetroGames)
 */

#ifndef RETROGAMELIB_H_
#define RETROGAMELIB_H_

#include "stm32f1xx_hal.h"
#include "main.h"
#include <stdbool.h>

#include "max7219.h"

#define delay(time) HAL_Delay(time)

#define MATRIX_NUM 5
#define ROW_NUM 24
#define COL_NUM 24

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

inline uint8_t matrix_num(uint8_t x, uint8_t y) {
	if (x > 7 && x < 16) {
		// центральний ряд матриць
		if (y < 8) {
			return 3;
		}
		if (y >= 8 && y < 16) {
			return 2;
		}
		if (y >= 16 && y < 24) {
			return 1;
		}
	} else if (x > 15) {
		return 0;
	}

	return 4;
}

enum {
	up = 1, right = 2, down = 3, left = 4
};

typedef struct Point {
	// оголош типу даних Point
	int row, col;
} Point;

int8_t scanJoystick(int8_t player);

void waitJoystic(); // common

void calibrate();

void win_msg(uint8_t player); // вивести повідомлення про перемогу

void draw_menu();

void choose_menu_item(char m, char choose); //блимання вибраного режиму в меню

void pong_score(int player1Score, int player2Score); //вивести рахунок (для pong)

void print_score(int score); //вивести рахунок (для змійки)

#endif /* RETROGAMELIB_H_ */
