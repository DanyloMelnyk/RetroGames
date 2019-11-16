#include "LedControl.h" // LedControl library is used for controlling a LED matrix. Find it using Library Manager or download zip here: https://github.com/wayoda/LedControl

#define MATRIX_NUM 3
#define ROW_NUM 8
#define COL_NUM 24

// --------------------------------------------------------------- //
// ------------------------- user config ------------------------- //
// --------------------------------------------------------------- //

// there are defined all the pins
struct Pin {
	static const short joystickX = A2;   // joystick X axis pin
	static const short joystickY = A3;   // joystick Y axis pin

	static const short potentiometer = A7; // potentiometer for snake speed control

	static const short CLK = 10;   // clock for LED matrix
	static const short CS  = 11;  // chip-select for LED matrix
	static const short DIN = 12; // data-in for LED matrix
};

// LED matrix brightness: between 0(darkest) and 15(brightest)
const short intensity = 8;

// lower = faster message scrolling
const short messageSpeed = 5;

// initial snake length (1...63, recommended 3)
const short initialSnakeLength = 3;


void setup() {
	Serial.begin(115200);  // set the same baud rate on your Serial Monitor
	initialize();         // initialize pins & LED matrix
	calibrateJoystick(); // calibrate the joystick home (do not touch it)
	showSnakeMessage(); // scrolls the 'snake' message around the matrix
}

// --------------------------------------------------------------- //
// -------------------- supporting variables --------------------- //
// --------------------------------------------------------------- //

LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, MATRIX_NUM);

struct Point {
	int row = 0, col = 0;
	Point(int row = 0, int col = 0): row(row), col(col) {}
};

struct Coordinate {
	int x = 0, y = 0;
	Coordinate(int x = 0, int y = 0): x(x), y(y) {}
};

void setLEDM(int row, int col, int v){
  if (col < 8 && col >= 0){
    matrix.setLed(2, row, col, v);
  } else if (col >= 8 && col < 16){
    matrix.setLed(1, row, col - 8, v);
  } else if (col >= 16 && col < 24){
    matrix.setLed(0, row, col - 16, v);
  }
}

bool win = false;
bool gameOver = false;

// primary snake head coordinates (snake head), it will be randomly generated
Point snake;

// food is not anywhere yet
Point food(-1, -1);

// construct with default values in case the user turns off the calibration
Coordinate joystickHome(500, 500);

// snake parameters
int snakeLength = initialSnakeLength; // choosed by the user in the config section
int snakeSpeed = 1; // will be set according to potentiometer value, cannot be 0
int snakeDirection = 0; // if it is 0, the snake does not move

// direction constants
const short up     = 1;
const short right  = 2;
const short down   = 3; // 'down - 2' must be 'up'
const short left   = 4; // 'left - 2' must be 'right'

// threshold where movement of the joystick will be accepted
const int joystickThreshold = 160;

// artificial logarithmity (steepness) of the potentiometer (-1 = linear, 1 = natural, bigger = steeper (recommended 0...1))
const float logarithmity = 0.4;

// snake body segments storage !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BOARD  !!!!!!!!!!!!!!!1
int gameboard[ROW_NUM][COL_NUM] = {};




// --------------------------------------------------------------- //
// -------------------------- functions -------------------------- //
// --------------------------------------------------------------- //

void loop() {
  generateFood();    // if there is no food, generate one

  long timestamp = millis();
 
  // intelligently blink with the food
  setLEDM(food.row, food.col, 1);
  
  scanJoystick();    // watches joystick movements & blinks with food
  calculateSnake();  // calculates snake parameters
  handleGameStates();

  // intelligently blink with the food
  if (millis() > 100 + timestamp){
    setLEDM(food.row, food.col, 0);
  } else {
    delay(100);
    setLEDM(food.row, food.col, 0);
  }

  // uncomment this if you want the current game board to be printed to the serial (slows down the game a bit)
  dumpGameBoard();
}




// if there is no food, generate one, also check for victory
void generateFood() {
	if (food.row == -1 || food.col == -1) {
		// self-explanatory
		if (snakeLength >= 64) {
			win = true;
			return; // prevent the food generator from running, in this case it would run forever, because it will not be able to find a pixel without a snake
		}

		// generate food until it is in the right position
		do {
			food.col = random(COL_NUM);
			food.row = random(ROW_NUM);
		} while (gameboard[food.row][food.col] > 0);
	}
}


// watches joystick movements & blinks with food
void scanJoystick() {
	int previousDirection = snakeDirection; // save the last direction
	long timestamp = millis();

	while (millis() < timestamp + snakeSpeed) {
		// calculate snake speed exponentially (10...1000ms)
		float raw = mapf(analogRead(Pin::potentiometer), 0, 1023, 0, 1);
		snakeSpeed = mapf(pow(raw, 3.5), 0, 1, 10, 1000); // change the speed exponentially
		if (snakeSpeed == 0) snakeSpeed = 1; // safety: speed can not be 0

		// determine the direction of the snake
		analogRead(Pin::joystickY) < joystickHome.y - joystickThreshold ? snakeDirection = down : 0; //up    : 0;
		analogRead(Pin::joystickY) > joystickHome.y + joystickThreshold ? snakeDirection = up : 0; //down  : 0;
		analogRead(Pin::joystickX) < joystickHome.x - joystickThreshold ? snakeDirection = left  : 0;
		analogRead(Pin::joystickX) > joystickHome.x + joystickThreshold ? snakeDirection = right : 0;

		// ignore directional change by 180 degrees (no effect for non-moving snake)
		snakeDirection + 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0;
		snakeDirection - 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0;

		// intelligently blink with the food
		//setLEDM(food.row, food.col, millis() % 100 < 50 ? 1 : 0);
	}
}


// calculate snake movement data
void calculateSnake() {
	switch (snakeDirection) {
		case up:
			snake.row--;
			fixEdge();
			setLEDM(snake.row, snake.col, 1);
			break;

		case right:
			snake.col++;
			fixEdge();
			setLEDM(snake.row, snake.col, 1);
			break;

		case down:
			snake.row++;
			fixEdge();
			setLEDM(snake.row, snake.col, 1);
			break;

		case left:
			snake.col--;
			fixEdge();
			setLEDM(snake.row, snake.col, 1);
			break;

		default: // if the snake is not moving, exit
			return;
	}

	// if there is a snake body segment, this will cause the end of the game (snake must be moving)
	if (gameboard[snake.row][snake.col] > 1 && snakeDirection != 0) {
		gameOver = true;
		return;
	}

	// check if the food was eaten
	if (snake.row == food.row && snake.col == food.col) {
		food.row = -1; // reset food
		food.col = -1;

		// increment snake length
		snakeLength++;

		// increment all the snake body segments
		for (int row = 0; row < ROW_NUM; row++) {
			for (int col = 0; col < COL_NUM; col++) {
				if (gameboard[row][col] > 0 ) {
					gameboard[row][col]++;
				}
			}
		}
	}

	// add new segment at the snake head location
	gameboard[snake.row][snake.col] = snakeLength + 1; // will be decremented in a moment

	// decrement all the snake body segments, if segment is 0, turn the corresponding led off
	for (int row = 0; row < ROW_NUM; row++) {
		for (int col = 0; col < COL_NUM; col++) {
			// if there is a body segment, decrement it's value
			if (gameboard[row][col] > 0 ) {
				gameboard[row][col]--;
			}

			// display the current pixel
			setLEDM(row, col, gameboard[row][col] == 0 ? 0 : 1);
		}
	}
}


// causes the snake to appear on the other side of the screen if it gets out of the edge
void fixEdge() {
	snake.col < 0 ? snake.col += COL_NUM : 0;
	snake.col > COL_NUM - 1 ? snake.col -= COL_NUM : 0;
	snake.row < 0 ? snake.row += ROW_NUM : 0;
	snake.row > ROW_NUM - 1 ? snake.row -= ROW_NUM : 0;
}


void handleGameStates() {
	if (gameOver || win) {
		unrollSnake();

		showScoreMessage(snakeLength - initialSnakeLength);

		if (gameOver) showGameOverMessage();
		else if (win) showWinMessage();

		// re-init the game
		win = false;
		gameOver = false;
		snake.row = random(ROW_NUM);
		snake.col = random(COL_NUM);
		food.row = -1;
		food.col = -1;
		snakeLength = initialSnakeLength;
		snakeDirection = 0;
		memset(gameboard, 0, sizeof(gameboard[0][0]) * COL_NUM * ROW_NUM);
		matrix.clearDisplay(0);
	}
}


void unrollSnake() {
	// switch off the food LED
	setLEDM(food.row, food.col, 0);

	delay(800);

	// flash the screen 5 times
	for (int i = 0; i < 5; i++) {
		// invert the screen
		for (int row = 0; row < ROW_NUM; row++) {
			for (int col = 0; col < COL_NUM; col++) {
				setLEDM(row, col, gameboard[row][col] == 0 ? 1 : 0);
			}
		}

		delay(20);

		// invert it back
		for (int row = 0; row < ROW_NUM; row++) {
			for (int col = 0; col < COL_NUM; col++) {
				setLEDM(row, col, gameboard[row][col] == 0 ? 0 : 1);
			}
		}

		delay(50);

	}


	delay(600);

	for (int i = 1; i <= snakeLength; i++) {
		for (int row = 0; row < ROW_NUM; row++) {
			for (int col = 0; col < COL_NUM; col++) {
				if (gameboard[row][col] == i) {
					setLEDM(row, col, 0);
					delay(100);
				}
			}
		}
	}
}


// calibrate the joystick home for 10 times
void calibrateJoystick() {
	Coordinate values;

	for (int i = 0; i < 10; i++) {
		values.x += analogRead(Pin::joystickX);
		values.y += analogRead(Pin::joystickY);
	}

	joystickHome.x = values.x / 10;
	joystickHome.y = values.y / 10;
}


void initialize() {
  for (int i = 0; i < MATRIX_NUM; i++){
    matrix.shutdown(i, false);
    matrix.setIntensity(i, intensity);
    matrix.clearDisplay(i);
  }
	
	randomSeed(analogRead(A5));
	snake.row = random(ROW_NUM);
	snake.col = random(COL_NUM);
}


void dumpGameBoard() {
	String buff = "\n\n\n";
	for (int row = 0; row < ROW_NUM; row++) {
		for (int col = 0; col < COL_NUM; col++) {
			if (gameboard[row][col] < 10) buff += " ";
			if (gameboard[row][col] != 0) buff += gameboard[row][col];
			else if (col == food.col && row == food.row) buff += "@";
			else buff += "-";
			buff += " ";
		}
		buff += "\n";
	}
	Serial.println(buff);
}





// --------------------------------------------------------------- //
// -------------------------- messages --------------------------- //
// --------------------------------------------------------------- //

const PROGMEM bool snakeMessage[8][73] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool gameOverMessage[8][90] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool scoreMessage[8][58] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool digits[][8][8] = {
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 1, 1, 1, 0},
		{0, 1, 1, 1, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 1, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 1, 1, 0, 0, 0, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 1, 1, 1, 0, 0},
		{0, 0, 1, 0, 1, 1, 0, 0},
		{0, 1, 0, 0, 1, 1, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 0, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	}
};


// scrolls the 'snake' message around the matrix
void showSnakeMessage() {
	[&] {
		//for (int d = 0; d < sizeof(snakeMessage[0]) - 7; d++) {
   for (int d = 0; d < 58; d++) {
			for (int col = 0; col < COL_NUM; col++) {
				delay(messageSpeed);
				for (int row = 0; row < ROW_NUM; row++) {
					// this reads the byte from the PROGMEM and displays it on the screen
					setLEDM(row, col, pgm_read_byte(&(snakeMessage[row][col + d])));
				}
			}

			// if the joystick is moved, exit the message
			if (analogRead(Pin::joystickY) < joystickHome.y - joystickThreshold
			        || analogRead(Pin::joystickY) > joystickHome.y + joystickThreshold
			        || analogRead(Pin::joystickX) < joystickHome.x - joystickThreshold
			        || analogRead(Pin::joystickX) > joystickHome.x + joystickThreshold) {
				return; // return the lambda function
			}
		}
	}();

  for (int i = 0; i < MATRIX_NUM; i++){
    matrix.clearDisplay(i);
  }

	// wait for joystick co come back
	while (analogRead(Pin::joystickY) < joystickHome.y - joystickThreshold
	        || analogRead(Pin::joystickY) > joystickHome.y + joystickThreshold
	        || analogRead(Pin::joystickX) < joystickHome.x - joystickThreshold
	        || analogRead(Pin::joystickX) > joystickHome.x + joystickThreshold) {}

}


// scrolls the 'game over' message around the matrix
void showGameOverMessage() {
	[&] {
		for (int d = 0; d < sizeof(gameOverMessage[0]) - 7; d++) {
			for (int col = 0; col < COL_NUM; col++) {
				delay(messageSpeed);
				for (int row = 0; row < ROW_NUM; row++) {
					// this reads the byte from the PROGMEM and displays it on the screen
					setLEDM(row, col, pgm_read_byte(&(gameOverMessage[row][col + d])));
				}
			}

			// if the joystick is moved, exit the message
			if (analogRead(Pin::joystickY) < joystickHome.y - joystickThreshold
			        || analogRead(Pin::joystickY) > joystickHome.y + joystickThreshold
			        || analogRead(Pin::joystickX) < joystickHome.x - joystickThreshold
			        || analogRead(Pin::joystickX) > joystickHome.x + joystickThreshold) {
				return; // return the lambda function
			}
		}
	}();

	for (int i = 0; i < MATRIX_NUM; i++){
    matrix.clearDisplay(i);
  }

	// wait for joystick co come back
	while (analogRead(Pin::joystickY) < joystickHome.y - joystickThreshold
	        || analogRead(Pin::joystickY) > joystickHome.y + joystickThreshold
	        || analogRead(Pin::joystickX) < joystickHome.x - joystickThreshold
	        || analogRead(Pin::joystickX) > joystickHome.x + joystickThreshold) {}

}


// scrolls the 'win' message around the matrix
void showWinMessage() {
	// not implemented yet // TODO: implement it
}


// scrolls the 'score' message with numbers around the matrix
void showScoreMessage(int score) {
	if (score < 0 || score > 99) return;

	// specify score digits
	int second = score % 10;
	int first = (score / 10) % 10;

	[&] {
		for (int d = 0; d < sizeof(scoreMessage[0]) + 2 * sizeof(digits[0][0]); d++) {
			for (int col = 0; col < COL_NUM; col++) {
				delay(messageSpeed);
				for (int row = 0; row < ROW_NUM; row++) {
					if (d <= sizeof(scoreMessage[0]) - 8) {
						setLEDM(row, col, pgm_read_byte(&(scoreMessage[row][col + d])));
					}

					int c = col + d - sizeof(scoreMessage[0]) + 6; // move 6 px in front of the previous message

					// if the score is < 10, shift out the first digit (zero)
					if (score < 10) c += 8;

					if (c >= 0 && c < 8) {
						if (first > 0) setLEDM(row, col, pgm_read_byte(&(digits[first][row][c]))); // show only if score is >= 10 (see above)
					} else {
						c -= 8;
						if (c >= 0 && c < 8) {
							setLEDM(row, col, pgm_read_byte(&(digits[second][row][c]))); // show always
						}
					}
				}
			}

			// if the joystick is moved, exit the message
			if (analogRead(Pin::joystickY) < joystickHome.y - joystickThreshold
			        || analogRead(Pin::joystickY) > joystickHome.y + joystickThreshold
			        || analogRead(Pin::joystickX) < joystickHome.x - joystickThreshold
			        || analogRead(Pin::joystickX) > joystickHome.x + joystickThreshold) {
				return; // return the lambda function
			}
		}
	}();

	for (int i = 0; i < MATRIX_NUM; i++){
    matrix.clearDisplay(i);
  }
}


// standard map function, but with floats
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
