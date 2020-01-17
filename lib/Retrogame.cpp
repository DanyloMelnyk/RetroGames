/*
  Retrogames.cpp - Library for work with MAX7219 8x8 matrix.
  Created by Danylo Melnyk(https://github.com/DanyloMelnyk) for Retro Games Cube(https://github.com/DanyloMelnyk/RetroGames), 01.11.2019.
*/

#include "Max7219.h"
#include "Retrogame.h"

//#define joystickY1 A2
//#define joystickX1 A3
//#define joystick1but 3
//
//#define joystickY2 A0
//#define joystickX2 A1
//#define joystick2but 2

void HardwareController::setLEDM(int row, int col, int v, bool upd)
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

void HardwareController::print_score(int score) ////// TODO !! rewrite to normal code 
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
					setLEDM(row + 8, col, pgm_read_byte(&(scoreMessage[row][col + d])), false);
				}

				int c = col + d - 66 + 6; // move 6 px in front of the previous message

				// if the score is < 10, shift out the first digit (zero)
				if (score < 10) c += 8;

				if (c >= 0 && c < 8)
				{
					if (first > 0) setLEDM(row + 8, col, pgm_read_byte(&(digits[first][row][c])), false);
					// show only if score is >= 10 (see above)
				}
				else
				{
					c -= 8;
					if (c >= 0 && c < 8)
					{
						setLEDM(row + 8, col, pgm_read_byte(&(digits[second][row][c])), false); // show always
					}
				}

				matrix.send();
			}
			
			if (j.scan(1) != -1 || j.scan(2) != -1)
			{
				matrix.clearDisplay();

				return;
			}
		}
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

int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
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
int tempo[] = {
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

int underworld_melody[] = {
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
//Underwolrd tempo
int underworld_tempo[] = {
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

void HardwareController::gameOverBuzz()
{
	int size = sizeof(underworld_melody) / sizeof(int);
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

	for (int row = 0; row < 8; row++)	// invert it back
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