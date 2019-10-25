#include "LedControl.h"

#define MATRIX_NUM 5
#define ROW_NUM 24
#define COL_NUM 24

int MODE = 1; // 0 - menu, 1 - snake, 2 - pong

struct Point
{
  int row = 0, col = 0;

  Point(int row = 0, int col = 0): row(row), col(col){}
};

struct Pin
{
  static const short joystickX1 = A2; // joystick X axis pin for player 1
  static const short joystickY1 = A3; // joystick Y axis pin for player 1

  static const short joystickX2 = A0; // joystick X axis pin for player 1
  static const short joystickY2 = A1; // joystick Y axis pin for player 1

  static const short potentiometer = A7; // potentiometer for snake speed control

  static const short CLK = 10; // clock for LED matrix
  static const short CS = 11; // chip-select for LED matrix
  static const short DIN = 12; // data-in for LED matrix
};

// LED matrix brightness: between 0(darkest) and 15(brightest)
const short intensity = 3;

// lower = faster message scrolling
const short messageSpeed = 1;

// construct with default values in case the user turns off the calibration
Point joystickHome1(500, 500);
Point joystickHome2(500, 500);

// threshold where movement of the joystick will be accepted
const int joystickThreshold = 160;

long time;

LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, 5);

////////// --------Pong----------//////
unsigned long lastRefreshTime = 0;
int refreshInterval = 1;
unsigned long lastMoveTime = 0;
int moveInterval;
unsigned long now = 0;
unsigned long overTime = 100000000;

int ballX;
int ballY;
int player1Score, player2Score;
int player1Position = 3, player2Position = 3; // позиція 1 пікселя ракетки поч з 8 зліва

int last_win = 0; // 0 - жоден, 1 - перший, 2 -другий

boolean ballMovingUp = true; // true - рух вверх, false - вниз
boolean ballMovingLeft = true; // true - наліво, false - направо
boolean straight = false; //true - ігнор ліво|право
boolean isGameOn = true;

int shape[24][8] = {
  // ігрове поле
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

///// ------ snake --------------/////

const short initialSnakeLength = 3;
bool win1 = false;
bool gameOver1 = false;
bool win2 = false;
bool gameOver2 = false;

// primary snake head coordinates (snake head), it will be randomly generated
Point snake1;
Point snake2;

// food is not anywhere yet
Point food(-1, -1);

// snake parameters
int snake1Length = initialSnakeLength; // choosed by the user in the config section for player 1
int snake2Length = initialSnakeLength; // choosed by the user in the config section for player 2
int snakeSpeed = 1; // will be set according to potentiometer value, cannot be 0
int snake1Direction = 0; // if it is 0, the snake does not move
int snake2Direction = 0; // if it is 0, the snake does not move

// direction constants
const short up = 1;
const short right = 2;
const short down = 3; // 'down - 2' must be 'up'
const short left = 4; // 'left - 2' must be 'right'

// snake body segments storage !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BOARD  !!!!!!!!!!!!!!!1
int gameboard[ROW_NUM][COL_NUM] = {};

//// common

void calibrateJoystick()
{
  Point values;

  for (int i = 0; i < 10; i++)
  {
    values.row += analogRead(Pin::joystickX1);
    values.col += analogRead(Pin::joystickY1);
  }

  joystickHome1.row = values.row / 10;
  joystickHome1.col = values.col / 10;

  values.row = 0;
  values.col = 0;

  for (int i = 0; i < 10; i++)
  {
    values.row += analogRead(Pin::joystickX2);
    values.col += analogRead(Pin::joystickY2);
  }

  joystickHome2.row = values.row / 10;
  joystickHome2.col = values.col / 10;
}

void scanJoystick()
{
  // Обробка джойстиків
  int Y1 = analogRead(Pin::joystickY1);
  int Y2 = analogRead(Pin::joystickY2);

  if (MODE == 2)
  {
    if (Y1 < joystickHome1.col - joystickThreshold && player1Position > 0) // down
    {
      player1Position--;
    }
    else if (Y1 > joystickHome1.col + joystickThreshold && player1Position < 5) // up
    {
      player1Position++;
    }

    if (Y2 < joystickHome2.col - joystickThreshold && player2Position < 5) // up
    {
      player2Position++;
    }
    else if (Y2 > joystickHome2.col + joystickThreshold && player2Position > 0) // down
    {
      player2Position--;
    }
  }
  else if (MODE == 1)
  {
   

    int previousDirection1 = snake1Direction; // save the last direction
    int previousDirection2 = snake2Direction; // save the last direction
    long timestamp = millis();

    while (millis() < timestamp + snakeSpeed)
    {
      int X1 = analogRead(Pin::joystickX1);
      int X2 = analogRead(Pin::joystickX2);

      int Y1 = analogRead(Pin::joystickY1);
      int Y2 = analogRead(Pin::joystickY2);
  
      // calculate snake speed exponentially (10...1000ms)
      float raw = mapf(analogRead(Pin::potentiometer), 0, 1023, 0, 1);
      snakeSpeed = mapf(pow(raw, 3.5), 0, 1, 10, 1000); // change the speed exponentially
      if (snakeSpeed == 0) snakeSpeed = 1; // safety: speed can not be 0

      // determine the direction of the snake
      Y1 < joystickHome1.col - joystickThreshold ? snake1Direction = down : 0; //up    : 0;
      Y1 > joystickHome1.col + joystickThreshold ? snake1Direction = up : 0; //down  : 0;
      X1 < joystickHome1.row - joystickThreshold ? snake1Direction = left : 0;
      X1 > joystickHome1.row + joystickThreshold ? snake1Direction = right : 0;

      // ignore directional change by 180 degrees (no effect for non-moving snake)
      snake1Direction + 2 == previousDirection1 && previousDirection1 != 0
        ? snake1Direction = previousDirection1
        : 0;
      snake1Direction - 2 == previousDirection1 && previousDirection1 != 0
        ? snake1Direction = previousDirection1
        : 0;

      Y2 < joystickHome2.col - joystickThreshold ? snake2Direction = down : 0; //up    : 0;
      Y2 > joystickHome2.col + joystickThreshold ? snake2Direction = up : 0; //down  : 0;
      X2 < joystickHome2.row - joystickThreshold ? snake2Direction = left : 0;
      X2 > joystickHome2.row + joystickThreshold ? snake2Direction = right : 0;

      // ignore directional change by 180 degrees (no effect for non-moving snake)
      snake2Direction + 2 == previousDirection2 && previousDirection2 != 0
        ? snake2Direction = previousDirection2
        : 0;
      snake2Direction - 2 == previousDirection2 && previousDirection2 != 0
        ? snake2Direction = previousDirection2
        : 0;
    }
  }
}

void setLEDM(int row, int col, int v) // COMMON
{
  if (row > 7 && row < 16)
  {
    // центральний ряд матриць
    if (col < 8 && col >= 0)
    {
      matrix.setLed(3, row - 8, col, v);
    }
    else if (col >= 8 && col < 16)
    {
      matrix.setLed(2, row - 8, col - 8, v);
    }
    else if (col >= 16 && col < 24)
    {
      matrix.setLed(1, row - 8, col - 16, v);
    }
  }
  else if (row > 15)
  {
    // нижня матриця
    matrix.setLed(0, row - 16, col - 8, v);
  }
  else
  {
    // верхня матриця
    matrix.setLed(4, row, col - 8, v);
  }
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//// main

void setup() //common
{
  for (int i = 0; i < 5; i++)
  {
    // ініціалізація матриць
    matrix.shutdown(i, false);
    matrix.setIntensity(i, intensity);
    matrix.clearDisplay(i);
  }

  Serial.begin(115200);

  calibrateJoystick();

  player1Score = 0;
  player2Score = 0;


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
  if (MODE == 2)
  {
    now = millis();
    if (isGameOn)
    {
      scanJoystick();
      update();
    }
    else
    {
      updateScore();
    }
    draw();
  }
  else if (MODE == 1)
  {
    generateFood(); // if there is no food, generate one

    while (millis() < 80 + time)
    {
    }

    // intelligently blink with the food
    setLEDM(food.row, food.col, 1);

    time = millis();

    do
    {
      scanJoystick(); // watches joystick movements
    }
    while (snake1Direction == 0 || snake2Direction == 0);

    calculateSnake(); // calculates snake parameters
    handleGameStates();

    // intelligently blink with the food
    while (millis() < 100 + time)
    {
    }

    setLEDM(food.row, food.col, 0);

    time = millis();

    // uncomment this if you want the current game board to be printed to the serial (slows down the game a bit)
    //dumpGameBoard();
  }
}

void buzz() // common
{
  //tone(speakerPin, 300, 20);
}

////////------- Pong ----/////

void gameOver() // pong
{
  isGameOn = false;
  overTime = now;

  for (int i = 0; i < 5; i++)
  {
    matrix.clearDisplay(i);
  }

  if (player1Score == 3)
  {
    first_win();
    last_win = 1;
    player1Score = 0;
    player2Score = 0;
  }
  if (player2Score == 3)
  {
    second_win();
    last_win = 2;
    player1Score = 0;
    player2Score = 0;
  }

  for (int i = 0; i < 5; i++)
  {
    matrix.clearDisplay(i);
  }

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
  isGameOn = true;
}

void updateBall() // pong
{
  if (!straight)
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

  if (ballY == 1 && ballX >= player2Position && ballX < player2Position + 3)
  {
    if (ballX > 2 && ballX < 6)
      ballX ++;

    ballMovingUp = false;
    moveInterval -= 20;
    //buzz();
  }
  else if (ballY == 22 && ballX >= player1Position && ballX < player1Position + 3)
  {
    if (ballX > 2 && ballX < 6)
      ballX ++;

    ballMovingUp = true;
    moveInterval -= 20;
    //buzz();
  }
}

void update() // pong
{
  if (now - lastMoveTime > moveInterval)
  {
    // очистка поля
    for (int i = 0; i < 24; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        shape[i][j] = 0;
      }
    }

    for (int i = 0; i < 8; i++)
    {
      if (i >= player1Position && i < player1Position + 3)
      {
        // відобразити 1 ракетку
        shape[23][i] = 1;
      }
      else
      {
        shape[23][i] = 0;
      }
      if (i >= player2Position && i < player2Position + 3)
      {
        // відобразити 2 ракетку
        shape[0][i] = 1;
      }
      else
      {
        shape[0][i] = 0;
      }
    }

    updateBall(); // переміщення м'яча
    shape[ballY][ballX] = 1; // відобразити м'яч

    lastMoveTime = now;
  }
}

void draw() // pong
{
  if (now - lastRefreshTime >= refreshInterval)
  {
    for (int i = 0; i < 24; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        setLEDM(i, j + 8, shape[i][j] == 1 ? HIGH : LOW);
      }
    }

    lastRefreshTime = now;
  }
}

////////////////
/// SNAKE
////////////////

void generateFood()
{
  if (food.row == -1 || food.col == -1)
  {
    // self-explanatory
    if (snake1Length >= 64)
    {
      win1 = true;
      return;
      // prevent the food generator from running, in this case it would run forever, because it will not be able to find a pixel without a snake
    }
    if (snake2Length >= 64)
    {
      win2 = true;
      return;
      // prevent the food generator from running, in this case it would run forever, because it will not be able to find a pixel without a snake
    }

    // generate food until it is in the right position
    do
    {
      food.col = random(COL_NUM);
      food.row = random(ROW_NUM);
    }
    while (gameboard[food.row][food.col] > 0 || (food.row < 8 && (food.col < 8 || food.col > 15)) || (food.row > 15
      && (food.col < 8 || food.col > 15)));
  }
}

// causes the snake to appear on the other side of the screen if it gets out of the edge

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

  ////////////////////

  snake1.col < 0 ? snake1.col += COL_NUM : 0;
  snake1.col > COL_NUM - 1 ? snake1.col -= COL_NUM : 0;
  snake1.row < 0 ? snake1.row += ROW_NUM : 0;
  snake1.row > ROW_NUM - 1 ? snake1.row -= ROW_NUM : 0;

  snake2.col < 0 ? snake2.col += COL_NUM : 0;
  snake2.col > COL_NUM - 1 ? snake2.col -= COL_NUM : 0;
  snake2.row < 0 ? snake2.row += ROW_NUM : 0;
  snake2.row > ROW_NUM - 1 ? snake2.row -= ROW_NUM : 0;
}

void initialize()
{
  snake1.row = 3;
  snake1.col = 11;

  snake2.row = 20;
  snake2.col = 11;
}

void calculateSnake()
{
  switch (snake1Direction)
  {
  case up:
    snake1.row--;
    fixEdge();
    setLEDM(snake1.row, snake1.col, 1);
    break;

  case right:
    snake1.col++;
    fixEdge();
    setLEDM(snake1.row, snake1.col, 1);
    break;

  case down:
    snake1.row++;
    fixEdge();
    setLEDM(snake1.row, snake1.col, 1);
    break;

  case left:
    snake1.col--;
    fixEdge();
    setLEDM(snake1.row, snake1.col, 1);
    break;

  default: // if the snake is not moving, exit
    return;
  }

  // if there is a snake body segment, this will cause the end of the game (snake must be moving)
  if ((gameboard[snake1.row][snake1.col] > 1 || (snake1.row == snake2.row && snake1.col == snake2.col)) &&
    snake1Direction != 0)
  {
    gameOver1 = true;
    return;
  }

  switch (snake2Direction)
  {
  case up:
    snake2.row--;
    fixEdge();
    setLEDM(snake2.row, snake2.col, 1);
    break;

  case right:
    snake2.col++;
    fixEdge();
    setLEDM(snake2.row, snake2.col, 1);
    break;

  case down:
    snake2.row++;
    fixEdge();
    setLEDM(snake2.row, snake2.col, 1);
    break;

  case left:
    snake2.col--;
    fixEdge();
    setLEDM(snake2.row, snake2.col, 1);
    break;

  default: // if the snake is not moving, exit
    return;
  }

  if ((gameboard[snake2.row][snake2.col] > 1 || (snake1.row == snake2.row && snake1.col == snake2.col)) &&
    snake2Direction != 0)
  {
    gameOver2 = true;
    return;
  }

  // check if the food was eaten
  if (snake1.row == food.row && snake1.col == food.col)
  {
    food.row = -1; // reset food
    food.col = -1;

    // increment snake length
    snake1Length++;
  }

  if (snake2.row == food.row && snake2.col == food.col)
  {
    food.row = -1; // reset food
    food.col = -1;

    // increment snake length
    snake2Length++;
  }

  // add new segment at the snake head location
  gameboard[snake1.row][snake1.col] = snake1Length + 1; // will be decremented in a moment
  gameboard[snake2.row][snake2.col] = snake2Length + 1; // will be decremented in a moment

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
      setLEDM(row, col, gameboard[row][col] == 0 ? 0 : 1);
    }
  }
}

void handleGameStates()
{
  if (gameOver1 || win1 || gameOver2 || win2)
  {
    unrollSnake();
    int score = 0;

    if (gameOver1 || win2)
    {
      second_win();
      score = snake2Length - initialSnakeLength;
    }
    else if (gameOver2 || win1)
    {
      first_win();
      score = snake1Length - initialSnakeLength;
    }

    showScoreMessage(score);

    // re-init the game
    win1 = false;
    gameOver1 = false;
    win2 = false;
    gameOver2 = false;

    snake1.row = 3;
    snake1.col = 11;

    do
    {
      snake2.row = 20;
      snake2.col = 11;
    }
    while (snake1.col == snake2.col && snake1.row == snake2.row);

    food.row = -1;
    food.col = -1;
    snake1Length = initialSnakeLength;
    snake1Direction = 0;

    snake2Length = initialSnakeLength;
    snake2Direction = 0;

    memset(gameboard, 0, sizeof(gameboard[0][0]) * COL_NUM * ROW_NUM);

    for (int i = 0; i < MATRIX_NUM; i++)
    {
      matrix.clearDisplay(i);
    }
  }
}

//////////////////
/// MSG
//////////////////
void waitJoystic();

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

const PROGMEM bool first_winMSG[8][97] = {
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,
    1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0,
    0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0
  }
};

const PROGMEM bool second_winMSG[8][97] = {
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
    0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,
    1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0,
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0
  }
};

const PROGMEM bool scoreMessage[8][66] = {
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0,
    0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1,
    0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }
};

void unrollSnake()
{
  // switch off the food LED
  setLEDM(food.row, food.col, 0);

  delay(800);

  // flash the screen 5 times
  for (int i = 0; i < 5; i++)
  {
    // invert the screen
    for (int row = 0; row < ROW_NUM; row++)
    {
      for (int col = 0; col < COL_NUM; col++)
      {
        setLEDM(row, col, gameboard[row][col] == 0 ? 1 : 0);
      }
    }

    delay(20);

    // invert it back
    for (int row = 0; row < ROW_NUM; row++)
    {
      for (int col = 0; col < COL_NUM; col++)
      {
        setLEDM(row, col, gameboard[row][col] == 0 ? 0 : 1);
      }
    }

    delay(50);
  }


  delay(600);

  for (int i = 1; i <= snake1Length + snake2Length; i++)
  {
    for (int row = 0; row < ROW_NUM; row++)
    {
      for (int col = 0; col < COL_NUM; col++)
      {
        if (gameboard[row][col] == i)
        {
          setLEDM(row, col, 0);
          delay(100);
        }
      }
    }
  }
}

void first_win() // common
{
  [&]
  {
    for (int d = 0; d < sizeof(second_winMSG[0]) - 16; d++)
    {
      for (int col = 0; col < COL_NUM; col++)
      {
        delay(1);
        for (int row = 0; row < 8; row++)
        {
          // this reads the byte from the PROGMEM and displays it on the screen
          setLEDM(row + 8, col, pgm_read_byte(&(first_winMSG[row][col + d])));
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(Pin::joystickY1) < joystickHome1.col - joystickThreshold
        || analogRead(Pin::joystickY1) > joystickHome1.col + joystickThreshold
        || analogRead(Pin::joystickX1) < joystickHome1.row - joystickThreshold
        || analogRead(Pin::joystickX1) > joystickHome1.row + joystickThreshold
        || analogRead(Pin::joystickY2) < joystickHome2.col - joystickThreshold
        || analogRead(Pin::joystickY2) > joystickHome2.col + joystickThreshold
        || analogRead(Pin::joystickX2) < joystickHome2.row - joystickThreshold
        || analogRead(Pin::joystickX2) > joystickHome2.row + joystickThreshold)
      {
        return; // return the lambda function
      }
    }
  }();

  for (int i = 0; i < 5; i++)
  {
    matrix.clearDisplay(i);
  }

  // wait for joystick co come back
  waitJoystic();
}

void second_win() // common
{
  [&]
  {
    for (int d = 0; d < sizeof(second_winMSG[0]) - 16; d++)
    {
      for (int col = 0; col < COL_NUM; col++)
      {
        delay(1);
        for (int row = 0; row < 8; row++)
        {
          // this reads the byte from the PROGMEM and displays it on the screen
          setLEDM(row + 8, col, pgm_read_byte(&(second_winMSG[row][col + d])));
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(Pin::joystickY1) < joystickHome1.col - joystickThreshold
        || analogRead(Pin::joystickY1) > joystickHome1.col + joystickThreshold
        || analogRead(Pin::joystickX1) < joystickHome1.row - joystickThreshold
        || analogRead(Pin::joystickX1) > joystickHome1.row + joystickThreshold
        || analogRead(Pin::joystickY2) < joystickHome2.col - joystickThreshold
        || analogRead(Pin::joystickY2) > joystickHome2.col + joystickThreshold
        || analogRead(Pin::joystickX2) < joystickHome2.row - joystickThreshold
        || analogRead(Pin::joystickX2) > joystickHome2.row + joystickThreshold)
      {
        return; // return the lambda function
      }
    }
  }();

  for (int i = 0; i < 5; i++)
  {
    matrix.clearDisplay(i);
  }

  // wait for joystick co come back
  waitJoystic();
}

void waitJoystic() // common
{
  while (analogRead(Pin::joystickY1) < joystickHome1.col - joystickThreshold
    || analogRead(Pin::joystickY1) > joystickHome1.col + joystickThreshold
    || analogRead(Pin::joystickX1) < joystickHome1.row - joystickThreshold
    || analogRead(Pin::joystickX1) > joystickHome1.row + joystickThreshold
    || analogRead(Pin::joystickY2) < joystickHome2.col - joystickThreshold
    || analogRead(Pin::joystickY2) > joystickHome2.col + joystickThreshold
    || analogRead(Pin::joystickX2) < joystickHome2.row - joystickThreshold
    || analogRead(Pin::joystickX2) > joystickHome2.row + joystickThreshold)
  {
  }
}

void updateScore() // pong
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      matrix.setLed(1, 7 - i, j, pgm_read_byte(&(digits[player2Score][j][i])));
      matrix.setLed(3, i, 7 - j, pgm_read_byte(&(digits[player1Score][j][i])));
    }
  }

  if (now - overTime > 3000)
    restartGame();
}

void showScoreMessage(int score)
{
  if (score < 0 || score > 99) return;

  // specify score digits
  int second = score % 10;
  int first = (score / 10) % 10;

  [&]
  {
    for (int d = 0; d < sizeof(scoreMessage[0]) + 2 * sizeof(digits[0][0]); d++)
    {
      for (int col = 0; col < COL_NUM; col++)
      {
        delay(messageSpeed);
        for (int row = 0; row < 8; row++)
        {
          if (d <= sizeof(scoreMessage[0]) - 8)
          {
            setLEDM(row + 8, col, pgm_read_byte(&(scoreMessage[row][col + d])));
          }

          int c = col + d - sizeof(scoreMessage[0]) + 6; // move 6 px in front of the previous message

          // if the score is < 10, shift out the first digit (zero)
          if (score < 10) c += 8;

          if (c >= 0 && c < 8)
          {
            if (first > 0) setLEDM(row + 8, col, pgm_read_byte(&(digits[first][row][c])));
            // show only if score is >= 10 (see above)
          }
          else
          {
            c -= 8;
            if (c >= 0 && c < 8)
            {
              setLEDM(row + 8, col, pgm_read_byte(&(digits[second][row][c]))); // show always
            }
          }
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(Pin::joystickY1) < joystickHome1.col - joystickThreshold
        || analogRead(Pin::joystickY1) > joystickHome1.col + joystickThreshold
        || analogRead(Pin::joystickX1) < joystickHome1.row - joystickThreshold
        || analogRead(Pin::joystickX1) > joystickHome1.row + joystickThreshold
        || analogRead(Pin::joystickY2) < joystickHome2.col - joystickThreshold
        || analogRead(Pin::joystickY2) > joystickHome2.col + joystickThreshold
        || analogRead(Pin::joystickX2) < joystickHome2.row - joystickThreshold
        || analogRead(Pin::joystickX2) > joystickHome2.row + joystickThreshold)
      {
        return; // return the lambda function
      }
    }
  }();

  for (int i = 0; i < MATRIX_NUM; i++)
  {
    matrix.clearDisplay(i);
  }
}
