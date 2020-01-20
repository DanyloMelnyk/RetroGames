// Pong.cpp
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 20.01.2020

#include "Pong.h"

void PongClass::scanJoystick() // Обробка джойстиків
{
#ifdef ENABLE_PAUSE
	if (digitalRead(joystick1but) == LOW && digitalRead(joystick2but) == LOW) // PAUSE
	{
		delay(1000);

		do
		{
			delay(1);
		}
		while (!(digitalRead(joystick1but) == LOW && digitalRead(joystick2but) == LOW));
	}
#endif

	int scan = controller.j.scan(1);

	if (scan == left && player1Position > 0)
	{
		player1Position--;
		for (int i = lastp1Position; i < lastp1Position + 3; i++)
			controller.matrix.setLed(0, 7, i, false);

		for (int i = player1Position; i < player1Position + 3; i++)
			controller.matrix.setLed(0, 7, i, true);

		lastp1Position = player1Position;
	}
	else if (scan == right && player1Position < 5)
	{
		player1Position++;

		for (int i = lastp1Position; i < lastp1Position + 3; i++)
			controller.matrix.setLed(0, 7, i, false);

		for (int i = player1Position; i < player1Position + 3; i++)
			controller.matrix.setLed(0, 7, i, true);

		lastp1Position = player1Position;
	}

	scan = controller.j.scan(2);

	if (scan == right && player2Position < 5)
	{
		player2Position++;

		for (int i = lastp2Position; i < lastp2Position + 3; i++)
			controller.matrix.setLed(4, 0, i, false);

		for (int i = player2Position; i < player2Position + 3; i++)
			controller.matrix.setLed(4, 0, i, true);

		lastp2Position = player2Position;
	}
	else if (scan == left && player2Position > 0)
	{
		player2Position--;
		for (int i = lastp2Position; i < lastp2Position + 3; i++)
			controller.matrix.setLed(4, 0, i, false);

		for (int i = player2Position; i < player2Position + 3; i++)
			controller.matrix.setLed(4, 0, i, true);

		lastp2Position = player2Position;
	}

	controller.matrix.send(0);
	controller.matrix.send(2);
	controller.matrix.send(4);
}

void PongClass::updateScore()
{
	controller.pong_score(player1Score, player2Score);

	if (now - overTime > 3000)
		restartGame();
}

void PongClass::initialize()
{
	player1Score = 0;
	player2Score = 0;

	updateScore();
}

void PongClass::loop()
{
	now = millis();

	if (isGameOn)
	{
		update();
		scanJoystick();
	}
	else
	{
		updateScore();
	}
}

void PongClass::gameOver()
{
	isGameOn = false;
	overTime = now;

	controller.matrix.clearDisplay();

	controller.gameOverBuzz();

	if (player1Score == 3)
	{
		controller.win_msg(1);
		last_win = 1;
		player1Score = 0;
		player2Score = 0;
	}

	if (player2Score == 3)
	{
		controller.win_msg(2);
		last_win = 2;
		player1Score = 0;
		player2Score = 0;
	}

	controller.matrix.clearDisplay();
}

void PongClass::restartGame()
{
	moveInterval = 300;

	if (last_win == 2)
	{
		// Подача від 2 гравця
		ballY = 17;
		ballMovingUp = true;
	}
	else // Подача від 1 гравця
	{
		ballY = 6;
		ballMovingUp = false;
	}

	ballX = now % 8;
	ballMovingLeft = true;

	for (int i = lastp1Position; i < lastp1Position + 3; i++)
		controller.matrix.setLed(0, 7, i, 0);

	for (int i = player1Position; i < player1Position + 3; i++)
		controller.matrix.setLed(0, 7, i, 1);

	lastp1Position = player1Position;

	for (int i = lastp2Position; i < lastp2Position + 3; i++)
		controller.matrix.setLed(4, 0, i, 0);

	for (int i = player2Position; i < player2Position + 3; i++)
		controller.matrix.setLed(4, 0, i, 1);

	controller.matrix.send();

	lastp2Position = player2Position;

	isGameOn = true;
}

void PongClass::updateBall()
{
	if (ballMovingLeft)
		ballX--;
	else
		ballX++;

	if (ballX <= 0) // зіткнення з лівою границею
	{
		ballMovingLeft = false;
		ballX = 0;
	}

	else if (ballX >= 7) // зіткнення з правою границею
	{
		ballMovingLeft = true;
		ballX = 7;
	}

	if (ballMovingUp)
	{
		if (ballY > 5 && ballY < 18)
			ballY -= random(2) + 1;
		else
			ballY--;
	}
	else
	{
		if (ballY > 5 && ballY < 18)
			ballY += random(2) + 1;
		else
			ballY++;
	}

	bool playBuzz = false;

	if (ballY == 1 && ballX >= player2Position && ballX < player2Position + 3)
	{
		ballMovingUp = false;
		moveInterval -= 10;
		playBuzz = true;
	}
	else if (ballY == 22 && ballX >= player1Position && ballX < player1Position + 3)
	{
		ballMovingUp = true;
		moveInterval -= 10;
		playBuzz = true;
	}

	if (ballY == 0)
	{
		player1Score++;
		last_win = 1;
		gameOver();
	}
	else if (ballY == 23)
	{
		player2Score++;
		last_win = 2;
		gameOver();
	}

	controller.setLEDM(lastballY, lastballX + 8, 0, false);
	controller.setLEDM(ballY, ballX + 8, 1, false);

	if (playBuzz)
		controller.buzz(2);
	else
		controller.buzz(1);

	lastballX = ballX;
	lastballY = ballY;
}

void PongClass::update()
{
	if (now - lastMoveTime > moveInterval * 1.1)
	{
		updateBall(); // переміщення м'яча

		controller.setLEDM(lastballY, lastballX + 8, 0, false); // знищ. минулої поз м'яча
		controller.setLEDM(ballY, ballX + 8, 1, false); // відображення м'яча

		controller.matrix.send(matrix_num(ballX, ballY));
		controller.matrix.send(matrix_num(lastballX, lastballY));

		lastballX = ballX;
		lastballY = ballY;

		lastMoveTime = now;
	}
}


// Pong.cpp
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 20.01.2020
