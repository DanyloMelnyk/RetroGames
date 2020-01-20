// Pong.h
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 20.01.2020

#ifndef _PONG_h
#define _PONG_h

#include "arduino.h"
#include "Retrogame.h"

extern HardwareController controller;

class PongClass
{
	//unsigned long lastRefreshTime = 0;
	//int refreshInterval = 1;
	unsigned long lastMoveTime = 0;
	unsigned long now = 0;
	unsigned long overTime = 100000000;
	int moveInterval;

	short ballX, ballY; // позиція м'яча
	short lastballX, lastballY; // попередня позиція м'яча
	char player1Score, player2Score; // рахунок
	short player1Position = 3, player2Position = 3; // позиція 1 пікселя ракетки поч з 8 зліва
	short lastp1Position, lastp2Position; // попередня позиція 1 пікселя ракетки поч з 8 зліва

	char last_win = 0; // 0 - жоден, 1 - перший, 2 -другий

	boolean ballMovingUp = true; // true - рух вверх, false - вниз
	boolean ballMovingLeft = true; // true - наліво, false - направо
	boolean isGameOn = true;

	void restartGame(); // функція запуску нової гри для pong
	void update(); // хід в pong
	void scanJoystick();
	void updateScore(); // pong
	void gameOver();
	void updateBall(); // pong

public:
	void initialize(); // функція запуску нової гри для snake
	void loop();

	PongClass()
	{
		initialize();
	}
};

#endif

// Pong.h
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 20.01.2020
