#include "LedControl.h"

#define ROW_NUM 24
#define COL_NUM 24

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

int shape[24][8] = { // ігрове поле
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

struct Pin {
  static const short joystickX1 = A2;   // joystick X axis pin for player 1
  static const short joystickY1 = A3;   // joystick Y axis pin for player 1

  static const short joystickX2 = A0;   // joystick X axis pin for player 1
  static const short joystickY2 = A1;   // joystick Y axis pin for player 1

  static const short potentiometer = A7; // potentiometer for snake speed control

  static const short CLK = 10;   // clock for LED matrix
  static const short CS  = 11;  // chip-select for LED matrix
  static const short DIN = 12; // data-in for LED matrix
};

struct Coordinate {
  int x = 0, y = 0;
  Coordinate(int x = 0, int y = 0): x(x), y(y) {}
};

// construct with default values in case the user turns off the calibration
Coordinate joystickHome1(500, 500);
Coordinate joystickHome2(500, 500);

// threshold where movement of the joystick will be accepted
const int joystickThreshold = 160;

LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, 5);

void calibrateJoystick() {
  Coordinate values;

  for (int i = 0; i < 10; i++) {
    values.x += analogRead(Pin::joystickX1);
    values.y += analogRead(Pin::joystickY1);
  }

  joystickHome1.x = values.x / 10;
  joystickHome1.y = values.y / 10;

  values.x = 0;
  values.y = 0;

  for (int i = 0; i < 10; i++) {
    values.x += analogRead(Pin::joystickX2);
    values.y += analogRead(Pin::joystickY2);
  }

  joystickHome2.x = values.x / 10;
  joystickHome2.y = values.y / 10;
}

void scanJoystick() { // Обробка джойстиків
  int Y1 = analogRead(Pin::joystickY1);
  int Y2 = analogRead(Pin::joystickY2);

  if (Y1 < joystickHome1.y - joystickThreshold && player1Position > 0) // down
  {
    player1Position--;
  }
  else if (Y1 > joystickHome1.y + joystickThreshold && player1Position < 5) // up
  {
    player1Position++;
  }

  if (Y2 < joystickHome2.y - joystickThreshold && player2Position < 5) // up
  {
    player2Position++;
  }
  else if (Y2 > joystickHome2.y + joystickThreshold && player2Position > 0) // down
  {
    player2Position--;
  }
}

void setup() {
  for (int i = 0; i < 5; i++) { // ініціалізація матриць
    matrix.shutdown(i, false);
    matrix.setIntensity(i, 5);
    matrix.clearDisplay(i);
  }

  calibrateJoystick();

  player1Score = 0;
  player2Score = 0;
  updateScore();
}

void loop() {
  now = millis();
  if (isGameOn) {
    scanJoystick();
    update();
  } else {
    updateScore();
  }
  draw();
}

void gameOver() {
  isGameOn = false;
  overTime = now;

    for (int i = 0; i < 5; i++) {
      matrix.clearDisplay(i);
    }

  if (player1Score == 3) {
    first_win();
    last_win = 1;
    player1Score = 0;
    player2Score = 0;
  }
  if (player2Score == 3) {
    second_win();
    last_win = 2;
    player1Score = 0;
    player2Score = 0;
  }

  for (int i = 0; i < 5; i++) {
    matrix.clearDisplay(i);
  }

  //int note[] = {700, 600, 500, 400, 300, 200};
  //for(int i = 0; i < 6; i++){
  //  tone(speakerPin, note[i], 150);
  //  delay(200);
  //}

}

void restartGame() {
  moveInterval = 300;
  
  if (last_win == 2){ // Подача від 2 гравця
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

void updateBall() {
  if (!straight) {
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
    if (ballY > 5)
      ballY -= random(2) + 1;
    else
      ballY--;
  }
  else
  {
    if (ballY < 18)
      ballY += random(2) + 1;
    else
      ballY++;
  }

  if (ballY == 0) {
    player1Score++;
    last_win = 1;
    gameOver();
  } else if (ballY == 23) {
    player2Score++;
    last_win = 2;
    gameOver();
  }

  if (ballY == 1 && ballX >= player2Position && ballX < player2Position + 3) {
    if (ballX > 2 && ballX < 6)
      ballX += 2 - random(3);

    ballMovingUp = false;
    moveInterval -= 20;
    //buzz();
  }
  else if (ballY == 22 && ballX >= player1Position && ballX < player1Position + 3) {
    if (ballX > 2 && ballX < 6)
      ballX += random(3) - 2;

    ballMovingUp = true;
    moveInterval -= 20;
    //buzz();
  }
}

void buzz() {
  tone(speakerPin, 300, 20);
}

void update() {
  if (now - lastMoveTime > moveInterval) { // очистка поля
    for (int i = 0; i < 24; i++) {
      for (int j = 0; j < 8; j++) {
        shape[i][j] = 0;
      }
    }

    for (int i = 0; i < 8; i++) {
      if (i >= player1Position && i < player1Position + 3) { // відобразити 1 ракетку
        shape[23][i] = 1;
      } else {
        shape[23][i] = 0;
      }
      if (i >= player2Position && i < player2Position + 3) { // відобразити 2 ракетку
        shape[0][i] = 1;
      } else {
        shape[0][i] = 0;
      }
    }

    updateBall(); // переміщення м'яча
    shape[ballY][ballX] = 1; // відобразити м'яч
    
    lastMoveTime = now;
  }
}

void setLEDM(int row, int col, int v) {
  if (row > 7 && row < 16) { // центральний ряд матриць
    if (col < 8 && col >= 0) {
      matrix.setLed(3, row - 8, col, v);
    }
    else if (col >= 8 && col < 16) {
      matrix.setLed(2, row - 8, col - 8, v);
    }
    else if (col >= 16 && col < 24) {
      matrix.setLed(1, row - 8, col - 16, v);
    }
  }
  else if (row > 15) { // нижня матриця
    matrix.setLed(0, row - 16, col - 8, v);
  }
  else { // верхня матриця
    matrix.setLed(4, row, col - 8, v);
  }
}

void draw() {
  if (now - lastRefreshTime >= refreshInterval) {
    for (int i = 0; i < 24; i++) {
      for (int j = 0; j < 8; j++) {
        setLEDM(i, j + 8, shape[i][j] == 1 ? HIGH : LOW);
      }
    }

    lastRefreshTime = now;
  }
}

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
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0}
};

const PROGMEM bool second_winMSG[8][97] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0}
};

void first_win() {
  [&] {
    for (int d = 0; d < sizeof(second_winMSG[0]) - 16; d++) {
      for (int col = 0; col < COL_NUM; col++) {
        delay(1);
        for (int row = 0; row < 8; row++) {
          // this reads the byte from the PROGMEM and displays it on the screen
          setLEDM(row + 8, col, pgm_read_byte(&(first_winMSG[row][col + d])));
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(Pin::joystickY1) < joystickHome1.y - joystickThreshold
          || analogRead(Pin::joystickY1) > joystickHome1.y + joystickThreshold
          || analogRead(Pin::joystickX1) < joystickHome1.x - joystickThreshold
          || analogRead(Pin::joystickX1) > joystickHome1.x + joystickThreshold
          || analogRead(Pin::joystickY2) < joystickHome2.y - joystickThreshold
          || analogRead(Pin::joystickY2) > joystickHome2.y + joystickThreshold
          || analogRead(Pin::joystickX2) < joystickHome2.x - joystickThreshold
          || analogRead(Pin::joystickX2) > joystickHome2.x + joystickThreshold) {
        return; // return the lambda function
      }
    }
  }();

  for (int i = 0; i < 5; i++) {
    matrix.clearDisplay(i);
  }

  // wait for joystick co come back
  waitJoystic();

}

void second_win() {
  [&] {
    for (int d = 0; d < sizeof(second_winMSG[0]) - 16; d++) {
      for (int col = 0; col < COL_NUM; col++) {
        delay(1);
        for (int row = 0; row < 8; row++) {
          // this reads the byte from the PROGMEM and displays it on the screen
          setLEDM(row + 8, col, pgm_read_byte(&(second_winMSG[row][col + d])));
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(Pin::joystickY1) < joystickHome1.y - joystickThreshold
          || analogRead(Pin::joystickY1) > joystickHome1.y + joystickThreshold
          || analogRead(Pin::joystickX1) < joystickHome1.x - joystickThreshold
          || analogRead(Pin::joystickX1) > joystickHome1.x + joystickThreshold
          || analogRead(Pin::joystickY2) < joystickHome2.y - joystickThreshold
          || analogRead(Pin::joystickY2) > joystickHome2.y + joystickThreshold
          || analogRead(Pin::joystickX2) < joystickHome2.x - joystickThreshold
          || analogRead(Pin::joystickX2) > joystickHome2.x + joystickThreshold) {
        return; // return the lambda function
      }
    }
  }();

  for (int i = 0; i < 5; i++) {
    matrix.clearDisplay(i);
  }

  // wait for joystick co come back
  waitJoystic();

}

void waitJoystic(){
  while (analogRead(Pin::joystickY1) < joystickHome1.y - joystickThreshold
         || analogRead(Pin::joystickY1) > joystickHome1.y + joystickThreshold
         || analogRead(Pin::joystickX1) < joystickHome1.x - joystickThreshold
         || analogRead(Pin::joystickX1) > joystickHome1.x + joystickThreshold
         || analogRead(Pin::joystickY2) < joystickHome2.y - joystickThreshold
         || analogRead(Pin::joystickY2) > joystickHome2.y + joystickThreshold
         || analogRead(Pin::joystickX2) < joystickHome2.x - joystickThreshold
         || analogRead(Pin::joystickX2) > joystickHome2.x + joystickThreshold) {}
}

void updateScore() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      matrix.setLed(1, 7 - i, j, pgm_read_byte(&(digits[player2Score][j][i])));
      matrix.setLed(3, i, 7 - j, pgm_read_byte(&(digits[player1Score][j][i])));
    }
  }

  if (now - overTime > 3000)
    restartGame();
}
