#include "Max7219.h"
#include "Retrogame.h"

#define ENABLE_PAUSE // дозволити паузу
//#define SERIAL_DEBUG // вивід в послідовний порт (19200 бод)
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames

char MODE = 0; // 0 - menu, 1 - snake, 2 - pong

enum
{
	MENU = 0,
	SNAKE,
	PONG
};

HardwareController controller;

unsigned long time;

////////// --------Pong----------//////
unsigned long lastRefreshTime = 0;
int refreshInterval = 1;
unsigned long lastMoveTime = 0;
int moveInterval;
unsigned long now = 0;
unsigned long overTime = 100000000;

short ballX, ballY; // позиція м'яча
short lastballX, lastballY; // попередня позиція м'яча
char player1Score, player2Score; // рахунок
short player1Position = 3, player2Position = 3; // позиція 1 пікселя ракетки поч з 8 зліва
short lastp1Position, lastp2Position; // попередня позиція 1 пікселя ракетки поч з 8 зліва

char last_win = 0; // 0 - жоден, 1 - перший, 2 -другий

boolean ballMovingUp = true; // true - рух вверх, false - вниз
boolean ballMovingLeft = true; // true - наліво, false - направо
boolean isGameOn = true;

///// ------ snake --------------/////

const short initialSnakeLength = 3;
bool win1 = false;
bool gameOver1 = false;
bool win2 = false;
bool gameOver2 = false;
bool one_player = true;

int move; // номер ходу

Point snake1, snake2; // коорд голови змії
Point food(-1, -1); // коорд їжі

int snake1Length = initialSnakeLength; // довжина 1 змії
int snake2Length = initialSnakeLength; // довжина 2 змії
int snake1Direction = 0, snake2Direction = 0; // напрям змії (0 - змія не рухається)

unsigned char gameboard[ROW_NUM][COL_NUM] = {}; // ігрове поле

void restartGame(); // функція запуску нової гри для pong
void initialize(); // функція запуску нової гри для snake
void update(); // хід в pong
void generateFood(); // генерація нової їжі для snake
void calculateSnake(); // хід в snake
void handleGameStates(); // перевірка виграшу в snake
void unrollSnake(); // анімація зникнення змії

//// common

void scanJoystick() // Обробка джойстиків
{
#ifdef ENABLE_PAUSE
	if (digitalRead(joystick1but) == LOW && digitalRead(joystick2but) == LOW) // PAUSE
	{
		delay(1000);

		do
		{
			delay(1);
		} while (!(digitalRead(joystick1but) == LOW && digitalRead(joystick2but) == LOW));
	}
#endif

	if (MODE == 2) // Pong
	{
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

		//delay(100);
	}
	else if (MODE == 1) // Snake
	{
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
		} while (millis() < timestamp + moveInterval);
	}
}

void menu()
{
	char choose = 0, m = -1;

	controller.draw_menu();
	unsigned long timestamp = millis(); // зберегти поточний час

	do
	{
		int scan1 = controller.j.scan(1);
		int scan2 = controller.j.scan(2);
		if (scan1 == up || scan2 == up) // up
		{
			//choose = 4;
			//m = 4;
		}
		else if (scan1 == down || scan2 == down) // down
		{
			choose = 1;
			m = 0;
		}
		else if (scan1 == right || scan2 == right) // right
		{
			choose = 2;
			m = 1;
		}
		else if (scan1 == left || scan2 == left) // left
		{
			choose = 3;
			m = 3;
		}

		if (m != -1)
		{
			controller.choose_menu_item(m, choose);

			if (timestamp - 800 > 0)
			{
				controller.buzz(1);
				timestamp = millis();
			}
		}
	} while ((digitalRead(joystick1but) == HIGH && digitalRead(joystick2but) == HIGH) || choose == 0);

	switch (choose)
	{
	case 1:
		MODE = SNAKE;
		one_player = true;
		break;
	case 2:
		MODE = SNAKE;
		one_player = false;
		break;
	case 3:
		MODE = PONG;
		break;
	default:
		MODE = MENU;
	}

	controller.buzz(2);

	controller.matrix.clearDisplay();
}

void updateScore() // pong
{
	controller.pong_score(player1Score, player2Score);

	if (now - overTime > 3000)
		restartGame();
}

//// main

void setup() // common
{
	pinMode(joystick1but, INPUT);
	pinMode(joystick2but, INPUT);

	pinMode(8, OUTPUT); //buzzer

	digitalWrite(joystick1but, HIGH);
	digitalWrite(joystick2but, HIGH);

	player1Score = 0;
	player2Score = 0;

#ifdef SERIAL_DEBUG
	Serial.begin(19200);
#endif

	controller.j.calbrate();
	
	do
	{
		menu();
	} while (MODE == 0);

	controller.matrix.clearDisplay();

	if (MODE == 2) // pong
	{
		updateScore();
	}
	else if (MODE == 1) // snake
	{
		initialize();
	}
}

void loop() //common
{
	if (MODE == 2) // Pong
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
	else if (MODE == 1) // Snake
	{
		generateFood(); // if there is no food, generate one

		while (millis() < 100 + time)
		{
		}

		// intelligently blink with the food
		controller.setLEDM(food.row, food.col, 1);

		time = millis();

		if (!one_player)
		{
			// 2 players
			do
			{
				scanJoystick(); // watches joystick movements
			} while (snake1Direction == 0 || snake2Direction == 0);
		}
		else // 1 player
		{
			do
			{
				scanJoystick(); // watches joystick movements
			} while (snake1Direction == 0);
		}

		calculateSnake(); // calculates snake parameters
		handleGameStates();

		// intelligently blink with the food
		while (millis() < 140 + time)
		{
		}

		controller.setLEDM(food.row, food.col, 0);

		time = millis();

		move++;

		if (move % 20 == 0) moveInterval -= 10;

		delay(10);
	}
}

////////------- Pong ----/////

void gameOver() // pong
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

	//int note[] = {700, 600, 500, 400, 300, 200};
	//for(int i = 0; i < 6; i++){
	//  tone(speakerPin, note[i], 150);
	//  delay(200);
	//}
}

void restartGame() // pong
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

void updateBall() // pong
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

void update() // pong
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

////////////////
/// SNAKE
////////////////

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
			food.col = random(COL_NUM);
			food.row = random(ROW_NUM);
		} while (gameboard[food.row][food.col] > 0 || (food.row < 8 && (food.col < 8 || food.col > 15)) || (food.row > 15
			&& (food.col < 8 || food.col > 15)));
	}
}

void fixEdge() // перехід між матрицями
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

void initialize() //snake
{
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

void calculateSnake() //snake
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
		{ // якщо вже щось є на даній позиції або зіткнення з іншою змійкою
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

void handleGameStates() //snake
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

void unrollSnake()
{
	// switch off the food LED
	controller.setLEDM(food.row, food.col, 0);

	delay(800);

	int note[] = { 700, 600, 500, 400, 300, 200 };
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

//Retro Games Cube 25.10.2019