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

#include <avr/pgmspace.h>

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class LedControl {
private:
	/* The array for shifting the data to the devices */
	byte spidata[16];
	/* Send out a single command to the device */
	void spiTransfer(int addr, byte opcode, byte data);

	/* We keep track of the led-status for all 8 devices in this array */
	byte status[64];
	/* Data is shifted out of this pin*/
	int SPI_MOSI;
	/* The clock is signaled on this pin */
	int SPI_CLK;
	/* This one is driven LOW for chip selectzion */
	int SPI_CS;
	/* The maximum number of devices we use */
	char maxDevices;

public:
	/*
	 * Create a new controler
	 * Params :
	 * dataPin		pin on the Arduino where data gets shifted out
	 * clockPin		pin for the clock
	 * csPin		pin for selecting the device
	 * numDevices	maximum number of devices that can be controled
	 */
	LedControl(int dataPin, int clkPin, int csPin, char numDevices = 1, char intensity = 8);

	/*
	 * Gets the number of devices attached to this LedControl.
	 * Returns :
	 * int	the number of devices on this LedControl
	 */
	int getDeviceCount() const;

	/*
	 * Set the shutdown (power saving) mode for the device
	 * Params :
	 * addr	The address of the display to control
	 * status	If true the device goes into power-down mode. Set to false
	 *		for normal operation.
	 */
	void shutdown(int addr, bool status);

	/*
	 * Set the number of digits (or rows) to be displayed.
	 * See datasheet for sideeffects of the scanlimit on the brightness
	 * of the display.
	 * Params :
	 * addr	address of the display to control
	 * limit	number of digits to be displayed (1..8)
	 */
	void setScanLimit(int addr, int limit);

	/*
	 * Set the brightness of the display.
	 * Params:
	 * addr		the address of the display to control
	 * intensity	the brightness of the display. (0..15)
	 */
	void setIntensity(int addr, char intensity);

	/*
	 * Switch all Leds on the display off.
	 * Params:
	 * addr	address of the display to control
	 */
	void clearDisplay(int addr);

	void clearDisplay();

	/*
	 * Set the status of a single Led.
	 * Params :
	 * addr	address of the display
	 * row	the row of the Led (0..7)
	 * col	the column of the Led (0..7)
	 * state	If true the led is switched on,
	 *		if false it is switched off
	 */
	void setLed(int addr, int row, int col, boolean state);

	//void setLedT(int addr, int row, int col, boolean state);

	void send(char matrix = -1);
};

#endif	//LedControl.h
