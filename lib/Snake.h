// Snake.h
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 20.01.2020

#ifndef _SNAKE_h
#define _SNAKE_h

#include "arduino.h"

#include "Retrogame.h"

extern HardwareController controller;

class SnakeClass
{
	const short initialSnakeLength = 3;
	bool win1 = false;
	bool gameOver1 = false;
	bool win2 = false;
	bool gameOver2 = false;
	bool one_player = true;
	bool food_state = false;

	int move = 0; // номер ходу

	Point snake1, snake2; // коорд голови змії
	Point food = Point(-1, -1); // коорд їжі

	int snake1Length = initialSnakeLength; // довжина 1 змії
	int snake2Length = initialSnakeLength; // довжина 2 змії
	int snake1Direction = 0, snake2Direction = 0; // напрям змії (0 - змія не рухається)
	int moveInterval = 400;

	unsigned char gameboard[ROW_NUM][COL_NUM] = {}; // ігрове поле

	void generateFood(); // генерація нової їжі для snake
	void calculateSnake(); // хід в snake
	void handleGameStates(); // перевірка виграшу в snake
	void unrollSnake(); // анімація зникнення змії
	void scanJoystick();
	void fixEdge();

public:
	void initialize(); // функція запуску нової гри для snake

	void setPlayerNum(char num)
	{
		if (num == 1)
			one_player = true;
		else
			one_player = false;
	}

	void loop();

	SnakeClass()
	{
		initialize();
	}
};
#endif

// Snake.h
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 20.01.2020
