/*
 *  Snake.h - snake game logic.
 *
 *  Created on: 20.01.2020
 *  Ported to STM32 on: 26.01.2020
 *  Author: Danylo Melnyk (https://github.com/DanyloMelnyk)
 *  Created as part of RGC project (https://github.com/DanyloMelnyk/RetroGames)
 */

#include <max7219.h>
#include <RetrogamesLib.h>
#include <Snake.h>

uint32_t time;

static const short initialSnakeLength = 3;
static bool win1; // = false;
static bool gameOver1; // = false;
static bool win2; // = false;
static bool gameOver2; // = false;
static bool one_player; // = true;
static bool food_state; // = false;

static int move; // = 0; // номер ходу

static Point snake1 = {0, 0}, snake2 = {0, 0}; // коорд голови змії
static Point food = {0, 0}; // = Point(-1, -1); // коорд їжі

static int snake1Length; // = initialSnakeLength; // довжина 1 змії
static int snake2Length; // = initialSnakeLength; // довжина 2 змії
static uint8_t snake1Direction; // = 0,
static uint8_t snake2Direction; // = 0; // напрям змії (0 - змія не рухається)
static int moveInterval; // = 400;

static uint8_t gameboard[ROW_NUM][COL_NUM]; // = {}; // ігрове поле

void SnakeLoop()
{
	generateFood(); // if there is no food, generate one

	if (HAL_GetTick() >= time + 200)
	{
		setLEDM(food.row, food.col, !food_state, true);
		time = HAL_GetTick();
		food_state = !food_state;
	}

	if (!one_player)
	{
		// 2 players
		do
		{
			SnakeScanJoystick(); // watches joystick movements
		}
		while (snake1Direction == 0 || snake2Direction == 0);
	}
	else // 1 player
	{
		do
		{
			SnakeScanJoystick(); // watches joystick movements
		}
		while (snake1Direction == 0);
	}

	calculateSnake(); // calculates snake parameters
	handleGameStates();

	move++;

	if (move % 20 == 0) moveInterval -= 10;

	delay(40);
}

void SnakeScanJoystick()
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
	unsigned long timestamp = HAL_GetTick(); // зберегти поточний час

	do
	{
		char Direction1 = scanJoystick(1);

		if (!((Direction1 + 2 == previousDirection1 || Direction1 - 2 == previousDirection1) && previousDirection1
			!= 0) && Direction1 != -1) // ігнорувати поворот на 180 градусів
			snake1Direction = Direction1;

		if (!one_player)
		{
			char Direction2 = scanJoystick(2);

			if (!((Direction2 + 2 == previousDirection2 || Direction2 - 2 == previousDirection2) &&
				previousDirection2 != 0) && Direction2 != -1) // ігнорувати поворот на 180 градусів
				snake2Direction = Direction2;
		}
	}
	while (HAL_GetTick() < timestamp + moveInterval);
}

void generateFood()
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
			food.col = rand() % COL_NUM;
			food.row = rand() % ROW_NUM;
		}
		while (gameboard[food.row][food.col] > 0 || (food.row < 8 && (food.col < 8 || food.col > 15)) || (food.row > 15
			&& (food.col < 8 || food.col > 15)));
	}
}

void fixEdge()
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

void initialize()
{
	win1 = false;
	gameOver1 = false;
	win2 = false;
	gameOver2 = false;
	one_player = true;
	food_state = false;
	move = 0; // номер ходу

	food.row = -1;
	food.col = -1; // коорд їжі
	snake1Length = initialSnakeLength; // довжина 1 змії
	snake2Length = initialSnakeLength; // довжина 2 змії
	snake1Direction = 0,
	snake2Direction = 0; // напрям змії (0 - змія не рухається)
	moveInterval = 400;

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

void calculateSnake()
{
	switch (snake1Direction)
	{
	case up:
		snake1.row--;
		fixEdge();
		setLEDM(snake1.row, snake1.col, 1, false);
		break;

	case right:
		snake1.col++;
		fixEdge();
		setLEDM(snake1.row, snake1.col, 1, false);
		break;

	case down:
		snake1.row++;
		fixEdge();
		setLEDM(snake1.row, snake1.col, 1, false);
		break;

	case left:
		snake1.col--;
		fixEdge();
		setLEDM(snake1.row, snake1.col, 1, false);
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
			setLEDM(snake2.row, snake2.col, 1, false);
			break;

		case right:
			snake2.col++;
			fixEdge();
			setLEDM(snake2.row, snake2.col, 1, false);
			break;

		case down:
			snake2.row++;
			fixEdge();
			setLEDM(snake2.row, snake2.col, 1, false);
			break;

		case left:
			snake2.col--;
			fixEdge();
			setLEDM(snake2.row, snake2.col, 1, false);
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
			setLEDM(row, col, gameboard[row][col] == 0 ? 0 : 1, false);
		}
	}

	//Controller.buzz(toBuzz);
	sendAll();
}

void handleGameStates() //snake
{
	if (gameOver1 || win1 || gameOver2 || win2)
	{
		unrollSnake();
//Controller.gameOverBuzz();
		int score = 0;

		if (gameOver1 || win2)
		{
			win_msg(2);
			score = snake2Length - initialSnakeLength;
		}
		else if (gameOver2 || win1)
		{
			win_msg(1);
			score = snake1Length - initialSnakeLength;
		}

		print_score(score);

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

		clearAllDisplay();
	}
}

void unrollSnake()
{
	// switch off the food LED
	setLEDM(food.row, food.col, 0, true);

	delay(800);

	//int note[] = {700, 600, 500, 400, 300, 200};

	// flash the screen 5 times
	for (int i = 0; i < 5; i++)
	{
		// invert the screen
		for (int row = 0; row < ROW_NUM; row++)
		{
			for (int col = 0; col < COL_NUM; col++)
			{
				setLEDM(row, col, gameboard[row][col] == 0 ? 1 : 0, false);
			}
		}

		sendAll();

		//delay(10);
		//tone(8, note[i], 150);

		// invert it back
		for (int row = 0; row < ROW_NUM; row++)
		{
			for (int col = 0; col < COL_NUM; col++)
			{
				setLEDM(row, col, gameboard[row][col] == 0 ? 0 : 1, false);
			}

			//Controller.matrix.send();
		}

		sendAll();


		delay(10);

		if (scanJoystick(1) != -1 || scanJoystick(2) != -1)
		{
			clearAllDisplay();

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
					setLEDM(row, col, 0, true);
					delay(100);
				}
			}
		}
	}
}

void SnakeSetup()
{
	calibrate();
	initialize();
}

void SnakeSetPlayerNum(uint8_t playerNum)
{
	setPlayerNum(playerNum);
}
