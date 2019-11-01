/*
  Retrogame.h - Library for work with MAX7219 8x8 matrix.
  Created by Danylo Melnyk(https://github.com/DanyloMelnyk), 01.11.2019.
*/

#ifndef RetroGamesLib
#define RetroGamesLib
#define MATRIX_NUM 5
#define ROW_NUM 24
#define COL_NUM 24

#define joystickY1 A2
#define joystickX1 A3
#define joystick1but 3

#define joystickY2 A0
#define joystickX2 A1
#define joystick2but 2

#include <Arduino.h>
#include "LedControl.h"

struct Point { // оголош типу даних Point
  int row = 0, col = 0;
  Point(int row = 0, int col = 0): row(row), col(col) {}
};

class Joystic
{
public:
	Point joystickHome1;
	Point joystickHome2;
	const int joystickThreshold = 160;

	void calibrateJoystick()
	{
		Point values;

		for (int i = 0; i < 10; i++)
		{
			values.row += analogRead(joystickX1);
			values.col += analogRead(joystickY1);
		}

		joystickHome1.row = values.row / 10;
		joystickHome1.col = values.col / 10;

		values.row = 0;
		values.col = 0;

		for (int i = 0; i < 10; i++)
		{
			values.row += analogRead(joystickX2);
			values.col += analogRead(joystickY2);
		}

		joystickHome2.row = values.row / 10;
		joystickHome2.col = values.col / 10;
	}

	void waitJoystic() // common
	{
		while (analogRead(joystickY1) < joystickHome1.col - joystickThreshold
			|| analogRead(joystickY1) > joystickHome1.col + joystickThreshold
			|| analogRead(joystickX1) < joystickHome1.row - joystickThreshold
			|| analogRead(joystickX1) > joystickHome1.row + joystickThreshold
			|| analogRead(joystickY2) < joystickHome2.col - joystickThreshold
			|| analogRead(joystickY2) > joystickHome2.col + joystickThreshold
			|| analogRead(joystickX2) < joystickHome2.row - joystickThreshold
			|| analogRead(joystickX2) > joystickHome2.row + joystickThreshold)
		{
		}
	}
};

void setLEDM(LedControl* matrix, int row, int col, int v); //встановлення стану діода(пікселя) на рядку row(нумерація зверху) і стовпці row(нум. зліва) на v(1-включений, 0 - викл)

void print_score(LedControl* matrix, int score, Joystic j); //вивести рахунок (для змійки)

void first_win(LedControl* matrix, Joystic j); //вивести повідомлення про перемогу 1 гравця

void second_win(LedControl* matrix, Joystic j); //вивести повідомлення про перемогу 2 гравця

void pong_score(LedControl* matrix, int player1Score, int player2Score); //вивести рахунок (для pong)

void choose_menu_item(LedControl* matrix, int m, int choose); //блимання вибраного режиму в меню

void draw_menu(LedControl* matrix); //вивести меню

#endif
