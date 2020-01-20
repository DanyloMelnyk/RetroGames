#include "Pong.h"
#include "Snake.h"
#include "Max7219.h"
#include "Retrogame.h"

#define ENABLE_PAUSE // дозволити паузу
//#define SERIAL_DEBUG // вивід в послідовний порт (19200 бод)

// lib.ino
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 25.10.2019

char MODE = 0; // 0 - menu, 1 - snake, 2 - pong

enum
{
	MENU = 0,
	SNAKE,
	PONG
};

unsigned long time;

HardwareController controller;
SnakeClass snake_game;
PongClass pong_game;

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
	}
	while ((digitalRead(joystick1but) == HIGH && digitalRead(joystick2but) == HIGH) || choose == 0);

	switch (choose)
	{
	case 1:
		MODE = SNAKE;
		snake_game.setPlayerNum(1);
		break;
	case 2:
		MODE = SNAKE;
		snake_game.setPlayerNum(2);
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

void setup()
{
	pinMode(joystick1but, INPUT);
	pinMode(joystick2but, INPUT);

	pinMode(8, OUTPUT); // buzzer

	digitalWrite(joystick1but, HIGH);
	digitalWrite(joystick2but, HIGH);

#ifdef SERIAL_DEBUG
	Serial.begin(19200);
#endif

	controller.j.calbrate();

	do
	{
		menu();
	}
	while (MODE == 0);

	controller.matrix.clearDisplay();

	if (MODE == 2) // pong
	{
		pong_game.initialize();
	}
	else if (MODE == 1) // snake
	{
		snake_game.initialize();
	}
}

void loop() // common
{
	if (MODE == PONG) // Pong
	{
		pong_game.loop();
	}
	else if (MODE == SNAKE) // Snake
	{
		snake_game.loop();
	}
}

// lib.ino
// Retro Games Cube
// https://github.com/DanyloMelnyk/RetroGames
// Danylo Melnyk 25.10.2019
