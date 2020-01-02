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
 */

#include "Max7219.h"

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

LedControl::LedControl(int dataPin, int clkPin, int csPin, char numDevices, char intensity)
{
	SPI_MOSI = dataPin;
	SPI_CLK = clkPin;
	SPI_CS = csPin;

	if (numDevices <= 0 || numDevices > 8)
		numDevices = 8;

	maxDevices = numDevices;

	pinMode(SPI_MOSI, OUTPUT);
	pinMode(SPI_CLK, OUTPUT);
	pinMode(SPI_CS, OUTPUT);
	digitalWrite(SPI_CS, HIGH);

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

int LedControl::getDeviceCount() const
{
	return maxDevices;
}

void LedControl::shutdown(int addr, bool b)
{
	if (addr < 0 || addr >= maxDevices)
		return;
	
	if (b)
		spiTransfer(addr, OP_SHUTDOWN, 0);
	else
		spiTransfer(addr, OP_SHUTDOWN, 1);
}

void LedControl::setScanLimit(int addr, int limit)
{
	if (addr < 0 || addr >= maxDevices)
		return;
	
	if (limit >= 0 && limit < 8)
		spiTransfer(addr, OP_SCANLIMIT, limit);
}

void LedControl::setIntensity(int addr, char intensity)
{
	if (addr < 0 || addr >= maxDevices)
		return;
	
	if (intensity >= 0 && intensity < 16)
		spiTransfer(addr, OP_INTENSITY, intensity);
}

void LedControl::clearDisplay(int addr)
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

void LedControl::clearDisplay()
{
	for (char i = 0; i < maxDevices; i++)
		clearDisplay(i);
}

//void LedControl::setLed(int addr, int row, int column, boolean state)	// матриця, рядок, стовпець, стан
//{
//	if (addr < 0 || addr >= maxDevices)
//		return;
//	if (row < 0 || row > 7 || column < 0 || column > 7)
//		return;
//
//	const int offset = addr * 8;
//	byte val = B10000000 >> column;
//	
//	if (state)
//	{
//		status[offset + row] = status[offset + row] | val;
//	}
//	else
//	{
//		val = ~val;
//		status[offset + row] = status[offset + row] & val;
//	}
//	
//	spiTransfer(addr, row + 1, status[offset + row]);
//}

void LedControl::setLed(int addr, int row, int column, boolean state)	// матриця, рядок, стовпець, стан
{
	if (addr < 0 || addr >= maxDevices)
		return;
	if (row < 0 || row > 7 || column < 0 || column > 7)
		return;

	const int offset = addr * 8;
	byte val = B10000000 >> column;

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

void LedControl::send(char matrix)
{
	if (matrix == -1)
	{
		for (char i = 0; i < maxDevices; i++)
			send(i);

		return;
	}

	const int offset = matrix * 8;
	
	for (int i = 0; i < 8; i++)
		spiTransfer(matrix, i + 1, status[offset + i]);
}

void LedControl::spiTransfer(int addr, volatile byte opcode, volatile byte data)
{
	const int offset = addr * 2; // зміщення 16 біт * індекс матриці
	const int maxbytes = maxDevices * 2; // довжина пакету 16 біт (2 байт) * к-ксть матриць

	for (int i = 0; i < maxbytes; i++)
		spidata[i] = (byte)0; // занулення майб пакету

	spidata[offset + 1] = opcode; // адреса (D16(11)-D8) зміщена на offset
	spidata[offset] = data; // дані

	digitalWrite(SPI_CS, LOW); // дозвіл прийому

	for (int i = maxbytes; i > 0; i--)
		shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, spidata[i - 1]); // вивід по 8 біт

	digitalWrite(SPI_CS, HIGH);
}
