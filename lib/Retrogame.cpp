/*
  Retrogames.cpp - Library for work with MAX7219 8x8 matrix.
  Created by Danylo Melnyk(https://github.com/DanyloMelnyk) for Retro Games Cube(https://github.com/DanyloMelnyk/RetroGames), 01.11.2019.
*/

#include "Max7219.h"
#include "Retrogame.h"

void HardwareController::setLEDM(int row, int col, bool v, bool upd)
{
	char to_update = -1;
	if (row > 7 && row < 16)
	{
		// центральний ряд матриць
		if (col < 8 && col >= 0)
		{
			matrix.setLed(3, row - 8, col, v);
			to_update = 3;
		}
		else if (col >= 8 && col < 16)
		{
			matrix.setLed(2, row - 8, col - 8, v);
			to_update = 2;
		}
		else if (col >= 16 && col < 24)
		{
			matrix.setLed(1, row - 8, col - 16, v);
			to_update = 1;
		}
	}
	else if (row > 15)
	{
		// нижня матриця
		matrix.setLed(0, row - 16, col - 8, v);
		to_update = 0;
	}
	else
	{
		// верхня матриця
		matrix.setLed(4, row, col - 8, v);
		to_update = 4;
	}

	if (upd)
	{
		matrix.send(to_update);
	}
}

void HardwareController::print_score(int score)
{
	if (score < 0 || score > 999) return;

	char third = char(score % 10);
	char second = char((score / 10) % 10);
	char first = char((score / 100) % 10);

	char num_size;
	if (score > 99)
		num_size = 3;
	else if (score > 9)
		num_size = 2;
	else
		num_size = 1;

	for (unsigned int k = 0; k < sizeof(scoreMessage[0]) + 4 + (num_size * 8); k++) //  + num_size * 8
	{
		for (unsigned int i = 0; i < 8; i++)
		{
			for (unsigned int j = 0; j < 24; j++)
			{
				if (j + k < 8)
					setLEDM(i + 8, j, false, false); // Пропуск 8 пікселів 
				else if (j + k < sizeof(scoreMessage[0]) + 8)
					setLEDM(i + 8, j, pgm_read_byte(&scoreMessage[i][j + k - 8]), false); // Текст повідомлення
				else if (j + k < 63 + 8 * num_size)
				{
					if (j + k > sizeof(scoreMessage[0]) + 8 * (num_size))
						setLEDM(i + 8, j, pgm_read_byte(&digits[third][i][j + k - 63 - 8 * (num_size - 1)]), false);
						// Остання цифра рахунку
					else if (j + k > sizeof(scoreMessage[0]) + 8 * (num_size - 1))
						setLEDM(i + 8, j, pgm_read_byte(&digits[second][i][j + k - 63 - 8 * (num_size - 2)]), false);
					else
						setLEDM(i + 8, j, pgm_read_byte(&digits[first][i][j + k - 63 - 8 * (num_size - 3)]), false);
				}
				else
					setLEDM(i + 8, j, false, false);
			}
		}

		matrix.send();
		if (k > 10)
			if (j.scan(1) != -1 || j.scan(2) != -1)
			{
				matrix.clearDisplay();
				delay(10);

				return;
			}

		delay(40);
	}

	matrix.clearDisplay();
	j.waitJoystic();
}

void HardwareController::win_msg(char player)
{
	for (int k = 0; k < 98; k++)
	{
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 24; j++)
				if (j + k < 8)
					setLEDM(i + 8, j, false, false);
				else if (j + k < 16)
					setLEDM(i + 8, j, pgm_read_byte(&digits[player][i][j + k - 8]), false);
				else if (j + k < 110)
					setLEDM(i + 8, j, pgm_read_byte(&winMSG[i][j + k - 16]), false);
				else
					setLEDM(i + 8, j, false, false);
		}

		matrix.send();
		delay(50);
	}

	matrix.clearDisplay();

	j.waitJoystic();
}

const int melody[] = {
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
}

void HardwareController::choose_menu_item(char m, char choose)
{
	for (int row = 0; row < 8; row++) // invert
	{
		for (int col = 0; col < 8; col++)
		{
			matrix.setLed(m, row, col, !pgm_read_byte(&(icon[choose - 1][row][col])));
		}

		matrix.send(m);
	}

	delay(10);

	for (int row = 0; row < 8; row++) // invert it back
	{
		for (int col = 0; col < 8; col++)
		{
			matrix.setLed(m, row, col, pgm_read_byte(&(icon[choose - 1][row][col])));
		}

		matrix.send(m);
	}

	delay(100);
}

void HardwareController::pong_score(int player1Score, int player2Score)
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			matrix.setLed(1, 7 - i, j, pgm_read_byte(&(digits[player2Score][j][i])));
			matrix.setLed(3, i, 7 - j, pgm_read_byte(&(digits[player1Score][j][i])));
		}
		matrix.send();
	}
}
