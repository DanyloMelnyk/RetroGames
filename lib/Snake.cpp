// Snake.cpp
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 20.01.2020

#include "Snake.h"

extern unsigned long time;

void SnakeClass::loop()
{
	generateFood(); // if there is no food, generate one

#ifdef SERIAL_DEBUG
	Serial.print(time);
	Serial.print(" ");
	Serial.println(millis());
#endif


	if (millis() >= time)
	{
		controller.setLEDM(food.row, food.col, !food_state);
		time = millis();
		food_state = !food_state;
	}

	if (!one_player)
	{
		// 2 players
		do
		{
			scanJoystick(); // watches joystick movements
		}
		while (snake1Direction == 0 || snake2Direction == 0);
	}
	else // 1 player
	{
		do
		{
			scanJoystick(); // watches joystick movements
		}
		while (snake1Direction == 0);
	}

	calculateSnake(); // calculates snake parameters
	handleGameStates();

	move++;

	if (move % 20 == 0) moveInterval -= 10;

	delay(40);
}

void SnakeClass::scanJoystick()
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

	char previousDirection1 = snake1Direction; // зберегти останній напрям
	char previousDirection2 = snake2Direction;
	unsigned long timestamp = millis(); // зберегти поточний час

	do
	{
		char Direction1 = controller.j.scan(1);

		if (!((Direction1 + 2 == previousDirection1 || Direction1 - 2 == previousDirection1) && previousDirection1
			!= 0) && Direction1 != -1) // ігнорувати поворот на 180 градусів
			snake1Direction = Direction1;

		if (!one_player)
		{
			char Direction2 = controller.j.scan(2);

			if (!((Direction2 + 2 == previousDirection2 || Direction2 - 2 == previousDirection2) &&
				previousDirection2 != 0) && Direction2 != -1) // ігнорувати поворот на 180 градусів
				snake2Direction = Direction2;
		}
	}
	while (millis() < timestamp + moveInterval);
}

void SnakeClass::generateFood()
{
	if (food.row == -1 || food.col == -1) // немає їжі на полі
	{
		if (snake1Length >= 64)
		{
			win1 = true;
			return;
		}
		if (snake2Length >= 64)
		{
			win2 = true;
			return;
		}

		do // генерування випадкових коорд, доки вони не в межах поля
		{
			food.col = random(COL_NUM);
			food.row = random(ROW_NUM);
		}
		while (gameboard[food.row][food.col] > 0 || (food.row < 8 && (food.col < 8 || food.col > 15)) || (food.row > 15
			&& (food.col < 8 || food.col > 15)));
	}
}

void SnakeClass::fixEdge()
{
	if (snake1.col < 8)
	{
		// ліва част
		if (snake1.row < 8)
		{
			// I сектор
			if (snake1Direction == up)
			{
				snake1.row = snake1.col;
				snake1.col = 8;
				snake1Direction = right;
			}
			else
			{
				snake1.col = snake1.row;
				snake1.row = 8;
				snake1Direction = down;
			}
		}
		else if (snake1.row > 15)
		{
			// II сектор
			if (snake1Direction == down)
			{
				snake1.row = 23 - snake1.col;
				snake1.col = 8;
				snake1Direction = right;
			}
			else
			{
				snake1.col = 23 - snake1.row;
				snake1.row = 15;
				snake1Direction = up;
			}
		}
	}
	else if (snake1.col > 15)
	{
		// права част
		if (snake1.row < 8)
		{
			// III сектор
			if (snake1Direction == right)
			{
				snake1.col = 23 - snake1.row;
				snake1.row = 8;
				snake1Direction = down;
			}
			else
			{
				snake1.row = 23 - snake1.col;
				snake1.col = 15;
				snake1Direction = left;
			}
		}
		else if (snake1.row > 15)
		{
			// IV сектор
			if (snake1Direction == down)
			{
				snake1.row = snake1.col;
				snake1.col = 15;
				snake1Direction = left;
			}
			else
			{
				snake1.col = snake1.row;
				snake1.row = 15;
				snake1Direction = up;
			}
		}
	}

	if (!one_player)
	{
		if (snake2.col < 8)
		{
			// ліва част
			if (snake2.row < 8)
			{
				// I сектор
				if (snake2Direction == up)
				{
					snake2.row = snake2.col;
					snake2.col = 8;
					snake2Direction = right;
				}
				else
				{
					snake2.col = snake2.row;
					snake2.row = 8;
					snake2Direction = down;
				}
			}
			else if (snake2.row > 15)
			{
				// II сектор
				if (snake2Direction == down)
				{
					snake2.row = 23 - snake2.col;
					snake2.col = 8;
					snake2Direction = right;
				}
				else
				{
					snake2.col = 23 - snake2.row;
					snake2.row = 15;
					snake2Direction = up;
				}
			}
		}
		else if (snake2.col > 15)
		{
			// права част
			if (snake2.row < 8)
			{
				// III сектор
				if (snake2Direction == right)
				{
					snake2.col = 23 - snake2.row;
					snake2.row = 8;
					snake2Direction = down;
				}
				else
				{
					snake2.row = 23 - snake2.col;
					snake2.col = 15;
					snake2Direction = left;
				}
			}
			else if (snake2.row > 15)
			{
				// IV сектор
				if (snake2Direction == down)
				{
					snake2.row = snake2.col;
					snake2.col = 15;
					snake2Direction = left;
				}
				else
				{
					snake2.col = snake2.row;
					snake2.row = 15;
					snake2Direction = up;
				}
			}
		}
	}

	////////////////////

	snake1.col < 0 ? snake1.col += COL_NUM : 0;
	snake1.col > COL_NUM - 1 ? snake1.col -= COL_NUM : 0;
	snake1.row < 0 ? snake1.row += ROW_NUM : 0;
	snake1.row > ROW_NUM - 1 ? snake1.row -= ROW_NUM : 0;

	if (!one_player)
	{
		snake2.col < 0 ? snake2.col += COL_NUM : 0;
		snake2.col > COL_NUM - 1 ? snake2.col -= COL_NUM : 0;
		snake2.row < 0 ? snake2.row += ROW_NUM : 0;
		snake2.row > ROW_NUM - 1 ? snake2.row -= ROW_NUM : 0;
	}
}

void SnakeClass::initialize()
{
	time = 0;
	food_state = true;

	snake1.row = 20; // початкове полож 1 гравця
	snake1.col = 11;

	if (!one_player)
	{
		snake2.row = 3; // початкове полож 2 гравця
		snake2.col = 11;
	}
	else
	{
		snake2.row = -1;
		snake2.col = -1;
		snake2Direction = -1;
	}

	move = 0;
	moveInterval = 400;
}

void SnakeClass::calculateSnake()
{
	switch (snake1Direction)
	{
	case up:
		snake1.row--;
		fixEdge();
		controller.setLEDM(snake1.row, snake1.col, 1, false);
		break;

	case right:
		snake1.col++;
		fixEdge();
		controller.setLEDM(snake1.row, snake1.col, 1, false);
		break;

	case down:
		snake1.row++;
		fixEdge();
		controller.setLEDM(snake1.row, snake1.col, 1, false);
		break;

	case left:
		snake1.col--;
		fixEdge();
		controller.setLEDM(snake1.row, snake1.col, 1, false);
		break;

	default: // if the snake is not moving, exit
		return;
	}

	if ((gameboard[snake1.row][snake1.col] > 1) && snake1Direction != 0) // якщо вже щось є на даній позиції
	{
		gameOver1 = true;
		return;
	}

	if (!one_player)
	{
		if ((snake1.row == snake2.row && snake1.col == snake2.col)) // зіткнення 2 змій
		{
			gameOver1 = true;
			return;
		}

		switch (snake2Direction)
		{
		case up:
			snake2.row--;
			fixEdge();
			controller.setLEDM(snake2.row, snake2.col, 1, false);
			break;

		case right:
			snake2.col++;
			fixEdge();
			controller.setLEDM(snake2.row, snake2.col, 1, false);
			break;

		case down:
			snake2.row++;
			fixEdge();
			controller.setLEDM(snake2.row, snake2.col, 1, false);
			break;

		case left:
			snake2.col--;
			fixEdge();
			controller.setLEDM(snake2.row, snake2.col, 1, false);
			break;

		default:
			return;
		}

		if ((gameboard[snake2.row][snake2.col] > 1 || (snake1.row == snake2.row && snake1.col == snake2.col)) &&
			snake2Direction != 0)
		{
			// якщо вже щось є на даній позиції або зіткнення з іншою змійкою
			gameOver2 = true;
			return;
		}
	}

	int toBuzz = 1;
	if (snake1.row == food.row && snake1.col == food.col) // їжа зїдена
	{
		food.row = -1;
		food.col = -1;

		snake1Length++;
		moveInterval -= 30; // збільш швидкості
		toBuzz = 2;
	}

	if (snake2.row == food.row && snake2.col == food.col) // їжа зїдена
	{
		food.row = -1;
		food.col = -1;

		snake2Length++;
		moveInterval -= 30; // збільш швидкості
		toBuzz = 2;
	}

	// встановлення "голови" змії на полі
	gameboard[snake1.row][snake1.col] = snake1Length + 1;
	gameboard[snake2.row][snake2.col] = snake2Length + 1;

	// decrement all the snake body segments, if segment is 0, turn the corresponding led off
	for (int row = 0; row < ROW_NUM; row++)
	{
		for (int col = 0; col < COL_NUM; col++)
		{
			// if there is a body segment, decrement it's value
			if (gameboard[row][col] > 0)
			{
				gameboard[row][col]--;
			}

			// display the current pixel
			controller.setLEDM(row, col, gameboard[row][col] == 0 ? 0 : 1, false);
		}
	}

	controller.buzz(toBuzz);
	controller.matrix.send();
}

void SnakeClass::handleGameStates() //snake
{
	if (gameOver1 || win1 || gameOver2 || win2)
	{
		unrollSnake();
		controller.gameOverBuzz();
		int score = 0;

		if (gameOver1 || win2)
		{
			controller.win_msg(2);
			score = snake2Length - initialSnakeLength;
		}
		else if (gameOver2 || win1)
		{
			controller.win_msg(1);
			score = snake1Length - initialSnakeLength;
		}

		controller.print_score(score);

		// re-init the game
		win1 = false;
		gameOver1 = false;
		win2 = false;
		gameOver2 = false;

		snake1.row = 20; // початкове полож 1 гравця
		snake1.col = 11;

		if (!one_player)
		{
			snake2.row = 3; // початкове полож 2 гравця
			snake2.col = 11;
		}
		else
		{
			snake2.row = -1;
			snake2.col = -1;
			snake2Direction = -1;
		}

		food.row = -1;
		food.col = -1;
		snake1Length = initialSnakeLength;
		snake1Direction = 0;

		snake2Length = initialSnakeLength;
		snake2Direction = 0;

		memset(gameboard, 0, sizeof(gameboard[0][0]) * COL_NUM * ROW_NUM);

		controller.matrix.clearDisplay();
	}
}

void SnakeClass::unrollSnake()
{
	// switch off the food LED
	controller.setLEDM(food.row, food.col, 0);

	delay(800);

	int note[] = {700, 600, 500, 400, 300, 200};
	for (int i = 0; i < 6; i++)
	{
		delay(200);
	}

	// flash the screen 5 times
	for (int i = 0; i < 5; i++)
	{
		// invert the screen
		for (int row = 0; row < ROW_NUM; row++)
		{
			for (int col = 0; col < COL_NUM; col++)
			{
				controller.setLEDM(row, col, gameboard[row][col] == 0 ? 1 : 0, false);
			}
		}

		controller.matrix.send();

		//delay(10);
		tone(8, note[i], 150);

		// invert it back
		for (int row = 0; row < ROW_NUM; row++)
		{
			for (int col = 0; col < COL_NUM; col++)
			{
				controller.setLEDM(row, col, gameboard[row][col] == 0 ? 0 : 1, false);
			}

			//controller.matrix.send();
		}
		controller.matrix.send();


		delay(10);

		if (controller.j.scan(1) != -1 || controller.j.scan(2) != -1)
		{
			controller.matrix.clearDisplay();

			return;
		}
	}

	delay(100);

	for (int i = 1; i <= snake1Length + snake2Length; i++)
	{
		for (int row = 0; row < ROW_NUM; row++)
		{
			for (int col = 0; col < COL_NUM; col++)
			{
				if (gameboard[row][col] == i)
				{
					controller.setLEDM(row, col, 0);
					delay(100);
				}
			}
		}
	}
}

// Snake.cpp
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 20.01.2020
