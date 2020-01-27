/*
 *    LedControl.cpp - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 *
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 *
 *    This permission notice shall be included in all copies or
 *    substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 *
 *    01.01.2020 optimized by Danylo Melnyk
 *
 *    Ported to STM32 on: 26.01.2020
 *    Port author: Danylo Melnyk (https://github.com/DanyloMelnyk)
 *    Ported as part of RGC project (https://github.com/DanyloMelnyk/RetroGames)
 */

#include <max7219.h>

// адреси регістрів MAX7219
#define OP_NOOP   0 // не задіяно
#define OP_DIGIT0 1 // рядки
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODE  9 // режим декодування
#define OP_INTENSITY   10 // яскравість
#define OP_SCANLIMIT   11 // ввімк/вимк рядків
#define OP_SHUTDOWN    12 // енергозбереження
#define OP_DISPLAYTEST 15 // режим тестування

extern SPI_HandleTypeDef hspi1;

#define cs_set() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET)
#define cs_reset() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET)

uint8_t spidata[16] = { 0 };
uint8_t status[64];
int8_t maxDevices;

void matrixSetup(int8_t numDevices, int8_t intensity)
{
	if (numDevices <= 0 || numDevices > 8)
		numDevices = 8;

	maxDevices = numDevices;

	cs_set();

	for (int i = 0; i < 64; i++)
	{
		status[i] = 0x00;
	}

	for (int i = 0; i < maxDevices; i++)
	{
		spiTransfer(i, OP_DISPLAYTEST, 0); // вимкнути режим тестування
		setScanLimit(i, 7); // ввімкнути всі 8 рядків на матриці
		spiTransfer(i, OP_DECODE, 0); // робота без декодування(в драйвері)
		setIntensity(i, intensity); // яскравість
		clearDisplay(i);
		shutdown(i, false); // ввімкнути енергозбереження
	}
}

void shutdown(int8_t addr, bool b)
{
	if (addr < 0 || addr >= maxDevices)
		return;

	if (b)
		spiTransfer(addr, OP_SHUTDOWN, 0);
	else
		spiTransfer(addr, OP_SHUTDOWN, 1);
}

void setScanLimit(int8_t addr, int8_t limit)
{
	if (addr < 0 || addr >= maxDevices)
		return;

	if (limit >= 0 && limit < 8)
		spiTransfer(addr, OP_SCANLIMIT, limit);
}

void setIntensity(int8_t addr, int8_t intensity)
{
	if (addr < 0 || addr >= maxDevices)
		return;

	if (intensity >= 0 && intensity < 16)
		spiTransfer(addr, OP_INTENSITY, intensity);
}

void clearDisplay(int8_t addr)
{
	if (addr < 0 || addr >= maxDevices)
		return;

	const int offset = addr * 8;
	for (int i = 0; i < 8; i++)
	{
		status[offset + i] = 0;
		spiTransfer(addr, i + 1, status[offset + i]);
	}
}

void clearAllDisplay()
{
	for (int8_t i = 0; i < 5; i++)
		clearDisplay(i);
}

void setLed(int8_t addr, int row, int column, bool state)	// матриця, рядок, стовпець, стан
{
	if (addr < 0 || addr >= maxDevices)
		return;
	if (row < 0 || row > 7 || column < 0 || column > 7)
		return;

	const int offset = addr * 8;
	uint8_t val = 128 >> column;

	if (state)
	{
		status[offset + row] = status[offset + row] | val;
	}
	else
	{
		val = ~val;
		status[offset + row] = status[offset + row] & val;
	}
}

void setLEDM(int row, int col, bool v, bool upd) // встановлення стану діода(пікселя) на рядку row(нумерація зверху) і стовпці row(нум. зліва) на v(1-включений, 0 - викл)
		{
	char to_update = -1;
	if (row > 7 && row < 16) {
		// центральний ряд матриць
		if (col < 8 && col >= 0) {
			setLed(3, row - 8, col, v);
			to_update = 3;
		} else if (col >= 8 && col < 16) {
			setLed(2, row - 8, col - 8, v);
			to_update = 2;
		} else if (col >= 16 && col < 24) {
			setLed(1, row - 8, col - 16, v);
			to_update = 1;
		}
	} else if (row > 15) {
		// нижня матриця
		setLed(0, row - 16, col - 8, v);
		to_update = 0;
	} else {
		// верхня матриця
		setLed(4, row, col - 8, v);
		to_update = 4;
	}

	if (upd) {
		send(to_update);
	}
}

void send(int8_t matrix)
{
	const int offset = matrix * 8;

	for (int i = 0; i < 8; i++)
		spiTransfer(matrix, i + 1, status[offset + i]);
}

void sendAll()
{
	for (int8_t i = 0; i < MATRIX_NUM; i++)
		send(i);
}

void spiTransfer(int8_t addr, uint8_t opcode, uint8_t data)
{
	const int offset = addr * 2; // зміщення 16 біт * індекс матриці
	const int maxbytes = maxDevices * 2; // довжина пакету 16 біт (2 байт) * к-ксть матриць

	for (int i = 0; i < maxbytes; i++)
		spidata[i] = (uint8_t)0; // занулення майб пакету

	spidata[offset + 1] = opcode; // адреса (D16(11)-D8) зміщена на offset
	spidata[offset] = data; // дані

	uint8_t Buf[1] = {0};

	cs_set(); // дозвіл прийому

	for (int i = maxbytes; i > 0; i--)
	{
		Buf[0] = spidata[i - 1];
		HAL_SPI_Transmit (&hspi1, (uint8_t*)Buf, 1, 5000);
	}

	cs_reset();
}



// #include "max7219.h"
//
// uint8_t aTxBuf[1]={0};
// extern SPI_HandleTypeDef hspi1;
// char dg=8;
//
// #define cs_set() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET)
// #define cs_reset() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET)
//
// void Send_7219 (uint8_t rg, uint8_t dt)
// {
//     cs_set();
//     aTxBuf[0]=rg;
//     HAL_SPI_Transmit (&hspi1, (uint8_t*)aTxBuf, 1, 5000);
//     aTxBuf[0]=dt;
//     HAL_SPI_Transmit (&hspi1, (uint8_t*)aTxBuf, 1, 5000);
//     cs_reset();
// }
// //------------------------------------------------------
// void Clear_7219 (void)
// {
//     uint8_t i=dg;
//     do
//     {
//         Send_7219(i,0xF);//символ пустоты
//     } while (--i);
// }
// //------------------------------------------------------
// void Number_7219 (volatile long n)
// {
//     uint8_t ng=0;//переменная для минуса
//     if(n<0)
//     {
//         ng=1;
//         n*=-1;
//     }
//     uint8_t i=0;
//     do
//     {
//         Send_7219(++i,n%10);//символ цифры
//         n/=10;
//     } while(n);
//     if(ng)
//     {
//         Send_7219(i+1,0x0A);//символ -
//     }
// }
// //-------------------------------------------------------
// void Init_7219 (void)
// {
//         Send_7219(0x09,0xFF);//включим режим декодирования
//         Send_7219(0x0B,dg-1);//кол-во используемых разрядов
//         Send_7219(0x0A,0x02);//интенсивность свечения
//         Send_7219(0x0C,0x01);//включим индикатор
//         Clear_7219();
// }
