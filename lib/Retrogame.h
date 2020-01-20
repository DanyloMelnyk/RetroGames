/*
  Retrogames.h - Library for work with MAX7219 8x8 matrix.
  Created by Danylo Melnyk(https://github.com/DanyloMelnyk) for Retro Games Cube(https://github.com/DanyloMelnyk/RetroGames), 01.11.2019.
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
#include "Max7219.h"

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

class Pong;

inline char matrix_num(int x, int y)
{
	if (x > 7 && x < 16)
	{
		// центральний ряд матриць
		if (y < 8 && y >= 0)
		{
			return 3;
		}
		if (y >= 8 && y < 16)
		{
			return 2;
		}
		if (y >= 16 && y < 24)
		{
			return 1;
		}
	}
	else if (x > 15)
	{
		return 0;
	}

	return 4;
}

enum
{
	up = 1,
	right = 2,
	down = 3,
	left = 4
};

class Point
{
	// оголош типу даних Point
public:
	int row = 0, col = 0;

	Point(int row = 0, int col = 0)
	{
		this->row = row;
		this->col = col;
	}
};

class Joystic
{
public:
	Point joystickHome1;
	Point joystickHome2;
	const int joystickThreshold = 300;

	char scan(char player)
	{
		int X, Y;

#ifdef SERIAL_DEBUG
		Serial.print((int)player);
#endif

		if (player == 1)
		{
			X = analogRead(joystickX1);
			Y = analogRead(joystickY1);

#ifdef SERIAL_DEBUG
			Serial.print("-player X: ");
			Serial.print(X);
			Serial.print(" Y: ");
			Serial.print(Y);
			Serial.print(" Home X: ");
			Serial.print(joystickHome1.row);
			Serial.print(" Home Y: ");
			Serial.print(joystickHome1.col);
			Serial.print("\n");
#endif

			if (X < joystickHome1.row - joystickThreshold)
				return left;
			if (X > joystickHome1.row + joystickThreshold)
				return right;
			if (Y < joystickHome1.col - joystickThreshold)
				return down;
			if (Y > joystickHome1.col + joystickThreshold)
				return up;
		}
		else
		{
			X = analogRead(joystickX2);
			Y = analogRead(joystickY2);

#ifdef SERIAL_DEBUG
			Serial.print("-player X: ");
			Serial.print(X);
			Serial.print(" Y: ");
			Serial.print(Y);
			Serial.print(" Home X: ");
			Serial.print(joystickHome2.row);
			Serial.print(" Home Y: ");
			Serial.print(joystickHome2.col);
			Serial.print("\n");
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

	void waitJoystic() // common
	{
		while (scan(1) == -1 && scan(2) == -1)
		{
			delay(1);
		}
	}

	void calbrate()
	{
		Point values;

		for (int i = 0; i < 10; i++)
		{
			values.row += analogRead(joystickX1);
			values.col += analogRead(joystickY1);
		}
#ifdef SERIAL_DEBUG
		Serial.print(" Test ");
		Serial.println(values.row);
#endif

		joystickHome1.row = values.row / 10;
		joystickHome1.col = values.col / 10;

#ifdef SERIAL_DEBUG
		Serial.print("New 1-player home is ");
		Serial.print(joystickHome1.row);
		Serial.print(" ");
		Serial.print(joystickHome1.col);
		Serial.print("\n");
#endif

		values.row = 0;
		values.col = 0;

		for (int i = 0; i < 10; i++)
		{
			values.row += analogRead(joystickX2);
			values.col += analogRead(joystickY2);
		}

		joystickHome2.row = values.row / 10;
		joystickHome2.col = values.col / 10;

#ifdef SERIAL_DEBUG
		Serial.print("New 2-player home is ");
		Serial.print(joystickHome2.row);
		Serial.print(" ");
		Serial.print(joystickHome2.col);
		Serial.print("\n");
#endif
	}

	Joystic()
	{
		calbrate();
	}
};

#define MATRIX_NUM 5
#define ROW_NUM 24
#define COL_NUM 24

#define CLK 10
#define CS 11
#define DIN 12

const PROGMEM bool icon[3][8][8] = {
	{
		//Snake1,
		0, 1, 1, 0, 0, 0, 0, 0,
		1, 0, 0, 1, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 1,
		1, 0, 0, 1, 0, 0, 1, 1,
		0, 1, 1, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 1
	},
	{
		//Snake2
		0, 0, 0, 0, 1, 1, 0, 1,
		0, 0, 0, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 1, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 1, 1, 0, 0,
		1, 0, 0, 1, 0, 0, 1, 0,
		1, 0, 0, 1, 0, 0, 1, 0,
		0, 1, 1, 0, 0, 1, 0, 0,
	},
	{
		//Pong2
		0, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 1, 0, 0, 1,
		0, 0, 0, 0, 1, 0, 0, 1,
		0, 0, 0, 0, 0, 1, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 1, 0, 0, 0,
		1, 0, 1, 1, 0, 0, 0, 0,
	}
};

class HardwareController
{
public:
	LedControl matrix = LedControl(DIN, CLK, CS, 5, 6);
	Joystic j = Joystic();
	void setLEDM(int row, int col, bool v, bool upd = true);
	// встановлення стану діода(пікселя) на рядку row(нумерація зверху) і стовпці row(нум. зліва) на v(1-включений, 0 - викл)
	void win_msg(char player); // вивести повідомлення про перемогу

	LedControl getMatrix()
	{
		return matrix;
	}

	void buzz(char melody) // common
	{
		if (melody == 0)
		{
			int note[] = {700, 600, 500, 400, 300, 200};
			for (int i = 0; i < 6; i++)
			{
				tone(8, note[i], 150);
				delay(200);
			}
		}
		else
		{
			int t = melody * 300;
			tone(8, t, 20);
		}
	}

	void gameOverBuzz();

	void draw_menu()
	{
		matrix.clearDisplay();

		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				matrix.setLed(0, i, j, pgm_read_byte(&(icon[0][i][j])));
				matrix.setLed(1, i, j, pgm_read_byte(&(icon[1][i][j])));
				//matrix.setLed(2, i, j, pgm_read_byte(&(digits[0][i][j])));
				matrix.setLed(3, i, j, pgm_read_byte(&(icon[2][i][j])));
				//matrix.setLed(4, i, j, pgm_read_byte(&(digits[4][i][j])));
			}

			matrix.send();
		}
	}

	void choose_menu_item(char m, char choose); //блимання вибраного режиму в меню

	void pong_score(int player1Score, int player2Score); //вивести рахунок (для pong)

	void print_score(int score); //вивести рахунок (для змійки)
};

const PROGMEM bool digits[10][8][8] = {
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 1, 1, 1, 0},
		{0, 1, 1, 1, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 1, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 1, 1, 0, 0, 0, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 1, 1, 1, 0, 0},
		{0, 0, 1, 0, 1, 1, 0, 0},
		{0, 1, 0, 0, 1, 1, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 0, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	}
};

const PROGMEM bool winMSG[8][94] = {
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
		1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
		1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
		1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1,
		1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
		1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
		1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
		1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
		1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
		1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
};

const PROGMEM bool scoreMessage[8][55] = {
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1,
		0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
		1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
		1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1,
		0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0,
		0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1,
		0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1,
		1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}
};

#endif
