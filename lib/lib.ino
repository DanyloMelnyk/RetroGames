#include "LedControl.h"
#include "Retrogame.h"

#define MATRIX_NUM 5
#define ROW_NUM 24
#define COL_NUM 24

int MODE = 0; // 0 - menu, 1 - snake, 2 - pong

struct Pin
{
  static const short CLK = 10; // clock for LED matrix
  static const short CS = 11; // chip-select for LED matrix
  static const short DIN = 12; // data-in for LED matrix
};

// LED matrix brightness: between 0(darkest) and 15(brightest)
const short intensity = 12;

// lower = faster message scrolling
const short messageSpeed = 1;

// construct with default values in case the user turns off the calibration
Point joystickHome1(500, 500);
Point joystickHome2(500, 500);

// threshold where movement of the joystick will be accepted
const int joystickThreshold = 160;
Joystic j;

unsigned long time;

LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, 5);

////////// --------Pong----------//////
unsigned long lastRefreshTime = 0;
int refreshInterval = 1;
unsigned long lastMoveTime = 0;
int moveInterval;
unsigned long now = 0;
unsigned long overTime = 100000000;

short ballX;
short ballY;
short lastballX;
short lastballY;
int player1Score, player2Score;
short player1Position = 3, player2Position = 3; // позиція 1 пікселя ракетки поч з 8 зліва
short lastp1Position, lastp2Position; // позиція 1 пікселя ракетки поч з 8 зліва

int last_win = 0; // 0 - жоден, 1 - перший, 2 -другий

boolean ballMovingUp = true; // true - рух вверх, false - вниз
boolean ballMovingLeft = true; // true - наліво, false - направо
boolean straight = false; //true - ігнор ліво|право
boolean isGameOn = true;

///// ------ snake --------------/////

const short initialSnakeLength = 3;
bool win1 = false;
bool gameOver1 = false;
bool win2 = false;
bool gameOver2 = false;
bool one_player = true;

int move;

// primary snake head coordinates (snake head), it will be randomly generated
Point snake1;
Point snake2;

Point food(-1, -1); // food is not anywhere yet

// snake parameters
int snake1Length = initialSnakeLength; // choosed by the user in the config section for player 1
int snake2Length = initialSnakeLength; // choosed by the user in the config section for player 2
int snake1Direction = 0; // if it is 0, the snake does not move
int snake2Direction = 0; // if it is 0, the snake does not move

// direction constants
const short up = 1;
const short right = 2;
const short down = 3; // 'down - 2' must be 'up'
const short left = 4; // 'left - 2' must be 'right'

// snake body segments storage
unsigned char gameboard[ROW_NUM][COL_NUM] = {};

// functions
void restartGame();
void initialize();
void update();
void generateFood();
void calculateSnake();
void handleGameStates();
void unrollSnake();

//// common

void scanJoystick()
{
  // Обробка джойстиків
  int Y1 = analogRead(joystickY1);
  int Y2 = analogRead(joystickY2);

  if (digitalRead(joystick1but) == LOW && digitalRead(joystick2but) == LOW)
  {
    delay(1000);

    do
    {
      delay(1);
    }
    while (!(digitalRead(joystick1but) == LOW && digitalRead(joystick2but) == LOW));

    //setup();
    //return;
  }

  if (MODE == 2) // Pong
  {
    if (Y1 < joystickHome1.col - joystickThreshold && player1Position > 0) // down
    {
      player1Position--;
    	for (int i = lastp1Position; i < lastp1Position + 3; i++)
        matrix.setLed(0, 7, i, 0);

      for (int i = player1Position; i < player1Position + 3; i++)
        matrix.setLed(0, 7, i, 1);

      lastp1Position = player1Position;
    }
    else if (Y1 > joystickHome1.col + joystickThreshold && player1Position < 5) // up
    {
      player1Position++;
    	for (int i = lastp1Position; i < lastp1Position + 3; i++)
        matrix.setLed(0, 7, i, 0);

      for (int i = player1Position; i < player1Position + 3; i++)
        matrix.setLed(0, 7, i, 1);

      lastp1Position = player1Position;
    }

    if (Y2 < joystickHome2.col - joystickThreshold && player2Position < 5) // up
    {
      player2Position++;
    	for (int i = lastp2Position; i < lastp2Position + 3; i++)
        matrix.setLed(4, 0, i, 0);

      for (int i = player2Position; i < player2Position + 3; i++)
        matrix.setLed(4, 0, i, 1);
      lastp2Position = player2Position;
    }
    else if (Y2 > joystickHome2.col + joystickThreshold && player2Position > 0) // down
    {
      player2Position--;
    	for (int i = lastp2Position; i < lastp2Position + 3; i++)
        matrix.setLed(4, 0, i, 0);

      for (int i = player2Position; i < player2Position + 3; i++)
        matrix.setLed(4, 0, i, 1);
      lastp2Position = player2Position;
    }

    delay(100);
  }
  else if (MODE == 1) // Snake
  {
    int previousDirection1 = snake1Direction; // save the last direction
    int previousDirection2 = snake2Direction; // save the last direction
    unsigned long timestamp = millis();

    //while (millis() < timestamp + snakeSpeed)
    do
    {
      int X1 = analogRead(joystickX1);
      int Y1 = analogRead(joystickY1);
      int X2, Y2;

      if (!one_player)
      {
        X2 = analogRead(joystickX2);
        Y2 = analogRead(joystickY2);
      }

      // determine the direction of the snake
      Y1 < joystickHome1.col - joystickThreshold ? snake1Direction = up : 0;
      Y1 > joystickHome1.col + joystickThreshold ? snake1Direction = down : 0;
      X1 < joystickHome1.row - joystickThreshold ? snake1Direction = right : 0; //left : 0;
      X1 > joystickHome1.row + joystickThreshold ? snake1Direction = left : 0; //right : 0;

      // ignore directional change by 180 degrees (no effect for non-moving snake)
      snake1Direction + 2 == previousDirection1 && previousDirection1 != 0
      ? snake1Direction = previousDirection1
                          : 0;
      snake1Direction - 2 == previousDirection1 && previousDirection1 != 0
      ? snake1Direction = previousDirection1
                          : 0;

      if (!one_player)
      {
        Y2 < joystickHome2.col - joystickThreshold ? snake2Direction = up : 0;
        Y2 > joystickHome2.col + joystickThreshold ? snake2Direction = down : 0;
        X2 < joystickHome2.row - joystickThreshold ? snake2Direction = right : 0; //left : 0;
        X2 > joystickHome2.row + joystickThreshold ? snake2Direction = left : 0; //right : 0;

        // ignore directional change by 180 degrees (no effect for non-moving snake)
        snake2Direction + 2 == previousDirection2 && previousDirection2 != 0
        ? snake2Direction = previousDirection2
                            : 0;
        snake2Direction - 2 == previousDirection2 && previousDirection2 != 0
        ? snake2Direction = previousDirection2
                            : 0;
      }
    } while (millis() < timestamp + moveInterval);
  }
}

void menu()
{
  int choose = 0, m = -1;

  for (int i = 0; i < 5; i++)
  {
    matrix.clearDisplay(i);
  }

	draw_menu(&matrix);

  do
  {
    int X1 = analogRead(joystickX1);
    int Y1 = analogRead(joystickY1);
    int X2 = analogRead(joystickX2);
    int Y2 = analogRead(joystickY2);

    if (Y1 < joystickHome1.col - joystickThreshold || Y2 < joystickHome2.col - joystickThreshold) // up
    {
      //choose = 4;
      //m = 4;
    }
    else if (Y1 > joystickHome1.col + joystickThreshold || Y2 > joystickHome2.col + joystickThreshold) // down
    {
      choose = 1;
      m = 0;
    }
    else if (X1 < joystickHome1.row - joystickThreshold || X2 < joystickHome2.row - joystickThreshold) // right
    {
      choose = 2;
      m = 1;
    }
    else if (X1 > joystickHome1.row + joystickThreshold || X2 > joystickHome2.row + joystickThreshold) // left
    {
      choose = 3;
      m = 3;
    }

    if (m != -1) {
      choose_menu_item(&matrix, m, choose);
    }
  }
  while ((digitalRead(joystick1but) == HIGH && digitalRead(joystick2but) == HIGH) || choose == 0);

  switch (choose)
  {
    case 1:
      MODE = 1;
      one_player = true;
      break;
    case 2:
      MODE = 1;
      one_player = false;
      break;
    case 3:
      MODE = 2;
      break;
    default:
      MODE = 0;
  }

  for (int i = 0; i < 5; i++)
  {
    matrix.clearDisplay(i);
  }
}

void updateScore() // pong
{
  pong_score(&matrix, player1Score, player2Score);

  if (now - overTime > 3000)
    restartGame();
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

  pinMode(joystick1but, INPUT);
  pinMode(joystick2but, INPUT);

  digitalWrite(joystick1but, HIGH);
  digitalWrite(joystick2but, HIGH);
  //Serial.begin(115200);

	j.calibrateJoystick();

  player1Score = 0;
  player2Score = 0;

  do
  {
    menu();
  } while (MODE == 0);


  for (int i = 0; i < 5; i++)
  {
    matrix.clearDisplay(i);
  }

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
      scanJoystick();
      update();
    }
    else
    {
      updateScore();
    }
  }
  else if (MODE == 1) // Snake
  {
    generateFood(); // if there is no food, generate one

    while (millis() < 80 + time)
    {
    }

    // intelligently blink with the food
    setLEDM(&matrix, food.row, food.col, 1);

    time = millis();

    if (!one_player) { // 2 players
      do
      {
        scanJoystick(); // watches joystick movements
      }
      while (snake1Direction == 0 || snake2Direction == 0);
    }
    else // 1 player
    {
      do
      {
        scanJoystick(); // watches joystick movements
      }
      while (snake1Direction == 0);
    }
    calculateSnake(); // calculates snake parameters
    handleGameStates();

    // intelligently blink with the food
    while (millis() < 100 + time)
    {
    }

    setLEDM(&matrix, food.row, food.col, 0);

    time = millis();

    move++;

    if (move % 20 == 0) moveInterval -= 10;

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
    first_win(&matrix, j);
    last_win = 1;
    player1Score = 0;
    player2Score = 0;
  }
  if (player2Score == 3)
  {
    second_win(&matrix, j);
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

  for (int i = lastp1Position; i < lastp1Position + 3; i++)
    matrix.setLed(0, 7, i, 0);

  for (int i = player1Position; i < player1Position + 3; i++)
    matrix.setLed(0, 7, i, 1);

  lastp1Position = player1Position;

  for (int i = lastp2Position; i < lastp2Position + 3; i++)
    matrix.setLed(4, 0, i, 0);

  for (int i = player2Position; i < player2Position + 3; i++)
    matrix.setLed(4, 0, i, 1);
  lastp2Position = player2Position;

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
  else
  {
    if (ballMovingUp)
      ballY--;
    else
      ballY++;

    straight = false;
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
    ballMovingUp = false;
    moveInterval -= 10;
    //buzz();
  }
  else if (ballY == 22 && ballX >= player1Position && ballX < player1Position + 3)
  {
    ballMovingUp = true;
    moveInterval -= 10;
    //buzz();
  }

  setLEDM(&matrix, lastballY, lastballX + 8, 0);
  setLEDM(&matrix, ballY, ballX + 8, 1);
  lastballX = ballX;
  lastballY = ballY;
}

void update() // pong
{
  if (now - lastMoveTime > moveInterval * 1.1)
  {
    updateBall(); // переміщення м'яча

    setLEDM(&matrix, lastballY, lastballX + 8, 0); // знищ. минулої поз м'яча
    setLEDM(&matrix, ballY, ballX + 8, 1); // відображення м'яча

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

void fixEdge()// causes the snake to appear on the other side of the screen if it gets out of the edge
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

void initialize()//snake
{
  snake1.row = 3;
  snake1.col = 11;

  if (!one_player)
  {
    snake2.row = 20;
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
      setLEDM(&matrix, snake1.row, snake1.col, 1);
      break;

    case right:
      snake1.col++;
      fixEdge();
      setLEDM(&matrix, snake1.row, snake1.col, 1);
      break;

    case down:
      snake1.row++;
      fixEdge();
      setLEDM(&matrix, snake1.row, snake1.col, 1);
      break;

    case left:
      snake1.col--;
      fixEdge();
      setLEDM(&matrix, snake1.row, snake1.col, 1);
      break;

    default: // if the snake is not moving, exit
      return;
  }

  // if there is a snake body segment, this will cause the end of the game (snake must be moving)
  if ((gameboard[snake1.row][snake1.col] > 1) && snake1Direction != 0)
  {
    gameOver1 = true;
    return;
  }
  if (!one_player) {
    if ((snake1.row == snake2.row && snake1.col == snake2.col))
    {
      gameOver1 = true;
      return;
    }
    switch (snake2Direction)
    {
      case up:
        snake2.row--;
        fixEdge();
        setLEDM(&matrix, snake2.row, snake2.col, 1);
        break;

      case right:
        snake2.col++;
        fixEdge();
        setLEDM(&matrix, snake2.row, snake2.col, 1);
        break;

      case down:
        snake2.row++;
        fixEdge();
        setLEDM(&matrix, snake2.row, snake2.col, 1);
        break;

      case left:
        snake2.col--;
        fixEdge();
        setLEDM(&matrix, snake2.row, snake2.col, 1);
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
  }



  // check if the food was eaten
  if (snake1.row == food.row && snake1.col == food.col)
  {
    food.row = -1; // reset food
    food.col = -1;

    // increment snake length
    snake1Length++;
    moveInterval -= 30;
  }

  if (snake2.row == food.row && snake2.col == food.col)
  {
    food.row = -1; // reset food
    food.col = -1;

    // increment snake length
    snake2Length++;
    moveInterval -= 30;
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
      setLEDM(&matrix, row, col, gameboard[row][col] == 0 ? 0 : 1);
    }
  }
}

void handleGameStates() //snake
{
  if (gameOver1 || win1 || gameOver2 || win2)
  {
    unrollSnake();
    int score = 0;

    if (gameOver1 || win2)
    {
      second_win(&matrix, j);
      score = snake2Length - initialSnakeLength;
    }
    else if (gameOver2 || win1)
    {
      first_win(&matrix, j);
      score = snake1Length - initialSnakeLength;
    }

    print_score(&matrix, score, j);

    // re-init the game
    win1 = false;
    gameOver1 = false;
    win2 = false;
    gameOver2 = false;

    snake1.row = 3;
    snake1.col = 11;

    if (!one_player) {
      snake2.row = 20;
      snake2.col = 11;
    }

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

void unrollSnake()
{
  // switch off the food LED
  setLEDM(&matrix, food.row, food.col, 0);

  delay(800);

  // flash the screen 5 times
  for (int i = 0; i < 5; i++)
  {
    // invert the screen
    for (int row = 0; row < ROW_NUM; row++)
    {
      for (int col = 0; col < COL_NUM; col++)
      {
        setLEDM(&matrix, row, col, gameboard[row][col] == 0 ? 1 : 0);
      }
    }

    delay(20);

    // invert it back
    for (int row = 0; row < ROW_NUM; row++)
    {
      for (int col = 0; col < COL_NUM; col++)
      {
        setLEDM(&matrix, row, col, gameboard[row][col] == 0 ? 0 : 1);
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
          setLEDM(&matrix, row, col, 0);
          delay(100);
        }
      }
    }
  }
}
//Danylo Melnyk 25.10.2019
