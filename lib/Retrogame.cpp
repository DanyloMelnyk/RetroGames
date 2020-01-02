﻿/*
  Retrogames.cpp - Library for work with MAX7219 8x8 matrix.
  Created by Danylo Melnyk(https://github.com/DanyloMelnyk) for Retro Games Cube(https://github.com/DanyloMelnyk/RetroGames), 01.11.2019.
*/

#include "Max7219.h"
#include "Retrogame.h"

#define joystickY1 A2
#define joystickX1 A3
#define joystick1but 3

#define joystickY2 A0
#define joystickX2 A1
#define joystick2but 2

void Joystic::calibrateJoystick()
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


void setLEDM(LedControl* matrix, int row, int col, int v, bool upd)
{
	if (row > 7 && row < 16)
	{
		// центральний ряд матриць
		if (col < 8 && col >= 0)
		{
			matrix->setLed(3, row - 8, col, v);
		}
		else if (col >= 8 && col < 16)
		{
			matrix->setLed(2, row - 8, col - 8, v);
		}
		else if (col >= 16 && col < 24)
		{
			matrix->setLed(1, row - 8, col - 16, v);
		}
	}
	else if (row > 15)
	{
		// нижня матриця
		matrix->setLed(0, row - 16, col - 8, v);
	}
	else
	{
		// верхня матриця
		matrix->setLed(4, row, col - 8, v);
	}

	if (upd)
	{
		matrix->send();
	}
}

void print_score(LedControl* matrix, int score, Joystic j)
{
	if (score < 0 || score > 99) return;

	// specify score digits
	int second = score % 10;
	int first = (score / 10) % 10;

	for (unsigned int d = 0; d < 66 + 2 * sizeof(digits[0][0]); d++)
	{
		for (int col = 0; col < 24; col++)
		{
			for (int row = 0; row < 8; row++)
			{
				if (d <= 66 - 8)
				{
					setLEDM(matrix, row + 8, col, pgm_read_byte(&(scoreMessage[row][col + d])), false);
				}

				int c = col + d - 66 + 6; // move 6 px in front of the previous message

				// if the score is < 10, shift out the first digit (zero)
				if (score < 10) c += 8;

				if (c >= 0 && c < 8)
				{
					if (first > 0) setLEDM(matrix, row + 8, col, pgm_read_byte(&(digits[first][row][c])), false);
					// show only if score is >= 10 (see above)
				}
				else
				{
					c -= 8;
					if (c >= 0 && c < 8)
					{
						setLEDM(matrix, row + 8, col, pgm_read_byte(&(digits[second][row][c])), false); // show always
					}
				}

				matrix->send();
			}

			matrix->send();
			
			if (j.scan(1) != -1 || j.scan(2) != -1)
			{
				matrix->clearDisplay();

				return;
			}
		}
	}

	matrix->clearDisplay();
	j.waitJoystic();
}

void first_win(LedControl* matrix, Joystic j)
{
	for (unsigned int d = 0; d < sizeof(second_winMSG[0]) - 16; d++)
	{
		for (int col = 0; col < 24; col++)
		{
			for (int row = 0; row < 8; row++)
			{
				setLEDM(matrix, row + 8, col, pgm_read_byte(&(first_winMSG[row][col + d])), false);
			}
			
			if (j.scan(1) != -1 || j.scan(2) != -1)
			{
				matrix->clearDisplay();
				return;
			}
		}

		matrix->send();
	}

	matrix->clearDisplay();
	
	j.waitJoystic();
}

void second_win(LedControl* matrix, Joystic j)
{
	for (unsigned int d = 0; d < sizeof(second_winMSG[0]) - 16; d++)
	{
		for (int col = 0; col < 24; col++)
		{
			for (int row = 0; row < 8; row++)
			{
				setLEDM(matrix, row + 8, col, pgm_read_byte(&(second_winMSG[row][col + d])), false);
			}
			
			if (j.scan(1) != -1 || j.scan(2) != -1)
			{
				matrix->clearDisplay();

				return;
			}
		}

		matrix->send();

	}

	matrix->clearDisplay();	

	j.waitJoystic();
}

void pong_score(LedControl* matrix, int player1Score, int player2Score)
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			matrix->setLed(1, 7 - i, j, pgm_read_byte(&(digits[player2Score][j][i])));
			matrix->setLed(3, i, 7 - j, pgm_read_byte(&(digits[player1Score][j][i])));
			matrix->send();
		}
	}
}

void choose_menu_item(LedControl* matrix, int m, int choose)
{
	for (int row = 0; row < 8; row++) // invert
	{
		for (int col = 0; col < 8; col++)
		{
			matrix->setLed(m, row, col, !pgm_read_byte(&(icon[choose - 1][row][col])));
		}

		//if (row % 2 == 0)
			matrix->send(m);

	}
	
	delay(10);

	for (int row = 0; row < 8; row++)	// invert it back
	{
		for (int col = 0; col < 8; col++)
		{
			matrix->setLed(m, row, col, pgm_read_byte(&(icon[choose - 1][row][col])));
		}

		//if (row % 2 == 0)
			matrix->send(m);
	}
	
	delay(100);
}

void draw_menu(LedControl* matrix)
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			matrix->setLed(0, i, j, pgm_read_byte(&(icon[0][i][j])));
			matrix->setLed(1, i, j, pgm_read_byte(&(icon[1][i][j])));
			//matrix.setLed(2, i, j, pgm_read_byte(&(digits[0][i][j])));
			matrix->setLed(3, i, j, pgm_read_byte(&(icon[2][i][j])));
			//matrix.setLed(4, i, j, pgm_read_byte(&(digits[4][i][j])));
		}

		matrix->send();
	}
}
