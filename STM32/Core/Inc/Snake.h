/*
 *  Snake.h - snake game logic.
 *
 *  Created on: 20.01.2020
 *  Ported to STM32 on: 26.01.2020
 *  Author: Danylo Melnyk (https://github.com/DanyloMelnyk)
 *  Created as part of RGC project (https://github.com/DanyloMelnyk/RetroGames)
 */

#ifndef SNAKE_H_
#define SNAKE_H_

#include "stm32f1xx_hal.h"
#include "main.h"
#include "RetrogamesLib.h"
#include <string.h>
#include <stdlib.h>

extern uint32_t time;

void generateFood(); // генерація нової їжі для snake
void calculateSnake(); // хід в snake
void handleGameStates(); // перевірка виграшу в snake
void unrollSnake(); // анімація зникнення змії
void SnakeScanJoystick();
void fixEdge();

void initialize(); // функція запуску нової гри для snake

void setPlayerNum(uint8_t num);

void SnakeLoop();

#endif /* SNAKE_H_ */
