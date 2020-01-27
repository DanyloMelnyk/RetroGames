/*
 *    LedControl.h - A library for controling Leds with a MAX7219/MAX7221
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

#ifndef Max7219
#define Max7219

#define MATRIX_NUM 5
#define ROW_NUM 24
#define COL_NUM 24

#include "stm32f1xx_hal.h"
#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#define delay(time) HAL_Delay(time)

/* Send out a single command to the device */
void spiTransfer(int8_t addr, uint8_t opcode, uint8_t data);

/*
 * Create a new controler
 * Params :
 * dataPin		pin on the Arduino where data gets shifted out
 * clockPin		pin for the clock
 * csPin		pin for selecting the device
 * numDevices	maximum number of devices that can be controled
 */
void matrixSetup(int8_t numDevices, int8_t intensity);

/*
 * Gets the number of devices attached to this LedControl.
 * Returns :
 * int	the number of devices on this LedControl
 */
int getDeviceCount();

/*
 * Set the shutdown (power saving) mode for the device
 * Params :
 * addr	The address of the display to control
 * status	If true the device goes into power-down mode. Set to false
 *		for normal operation.
 */
void shutdown(int8_t addr, bool status);

/*
 * Set the number of digits (or rows) to be displayed.
 * See datasheet for sideeffects of the scanlimit on the brightness
 * of the display.
 * Params :
 * addr	address of the display to control
 * limit	number of digits to be displayed (1..8)
 */
void setScanLimit(int8_t addr, int8_t limit);

/*
 * Set the brightness of the display.
 * Params:
 * addr		the address of the display to control
 * intensity	the brightness of the display. (0..15)
 */
void setIntensity(int8_t addr, int8_t intensity);

/*
 * Switch all Leds on the display off.
 * Params:
 * addr	address of the display to control
 */
void clearDisplay(int8_t addr);

void clearAllDisplay();

/*
 * Set the status of a single Led.
 * Params :
 * addr	address of the display
 * row	the row of the Led (0..7)
 * col	the column of the Led (0..7)
 * state	If true the led is switched on,
 *		if false it is switched off
 */
void setLed(int8_t addr, int row, int col, bool state);

void setLEDM(int row, int col, bool v, bool upd); // встановлення стану діода(пікселя) на рядку row(нумерація зверху) і стовпці row(нум. зліва) на v(1-включений, 0 - викл)


void send(int8_t matrix);

void sendAll();


#endif	//LedControl.h
