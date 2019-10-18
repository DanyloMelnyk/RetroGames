#include "LedControl.h"

unsigned long lastRefreshTime = 0;
int refreshInterval = 1;
unsigned long lastMoveTime = 0;
int moveInterval;
unsigned long now = 0;
unsigned long overTime = 100000000;
int currentColumn = 0;
int ballX;
int ballY;
int player1Score, player2Score;
int player1Position, player2Position;
boolean ballMovingUp = true;
boolean ballMovingLeft = true;
boolean straight = false;
boolean isGameOn = true;
int shape[24][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
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

LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, 5);

void setup(){
  for (int i = 0; i < 5; i++){
    matrix.shutdown(i, false);
    matrix.setIntensity(i, 5);
    matrix.clearDisplay(i);
  }
  
  delay(2000);
  //Serial.begin(9600);
  player1Score = 0; 
  player2Score = 0;
  restartGame();
}
 
void loop(){
  now = millis();
  if(isGameOn){
    update();
  }else{
    updateScore(); 
  }
  draw();
}

void gameOver(){
    isGameOn = false;
    overTime = now;
    player1Score %= 7;
    player2Score %= 7;
    //clearPins();
    
    //int note[] = {700, 600, 500, 400, 300, 200};
    //for(int i = 0; i < 6; i++){
    //  tone(speakerPin, note[i], 150);
    //  delay(200);
    //}
   
}

/*void clearPins(){
  for(int i = 0; i < 11; i++){
    digitalWrite(i, 0);
  }
}*/

void restartGame(){
  moveInterval = 300;
  ballX = now % 8;
  ballY = 6;
  ballMovingUp = false;
  ballMovingLeft = true;
  isGameOn = true;
}

void updateBall(){
  if (!straight){
  if(ballMovingLeft)
    ballX--;
  else
    ballX++;
    
  if(ballX <= 0)
  {
    ballMovingLeft = false;
    ballX = 0;
  }
    
  else if (ballX >= 7)
  {
     ballMovingLeft = true;
     ballX = 7;
  }
   
  }
    
  if(ballMovingUp)
  {
    if (ballY > 2)
        ballY -= random(2) + 1;
    else        
      ballY--;
  }
  else
  {
    if (ballY < 21)
        ballY += random(2) + 1;
    else        
      ballY++;
  }
  
  if(ballY == 0){
    player1Score++;
    gameOver();
  }else if (ballY == 23){
    player2Score++;
    gameOver();
  }
  
  if(ballY == 1 && ballX >= player2Position && ballX < player2Position + 3){
    if (ballX > 1 && ballX < 22)
      ballX -= random(5) - 2;
      
    ballMovingUp = false;
    moveInterval -= 20;
    //buzz();
  }
  else if(ballY == 22 && ballX >= player1Position && ballX < player1Position + 3){
    if (ballX > 1 && ballX < 22)
      ballX += random(5) - 2;
    
    ballMovingUp = true;
    moveInterval -= 20;
    //buzz();
  }
      
}

void buzz(){
  //tone(speakerPin, 300, 20); 
}

void update(){
  //clear table;
  if(now - lastMoveTime > moveInterval){
    for(int i = 0; i < 24; i++){
      for(int j = 0; j < 8; j++){
        shape[i][j] = 0;
      }
    }
    
    //update player positions
    int player1PotansValue = analogRead(A3);
    int player2PotansValue = analogRead(A1);
    //Serial.println(player2PotansValue);
    player1Position = player1PotansValue * 6 / 1024;
    player2Position = player2PotansValue * 6 / 1024;
    for(int i = 0; i < 8; i++){
      if(i >= player1Position && i < player1Position + 3){
        shape[23][i] = 1;
      }else{
        shape[23][i] = 0;
      }
      if(i >= player2Position && i < player2Position + 3){
        shape[0][i] = 1;
      }else{
        shape[0][i] = 0;
      }
    }
    //update the ball
    updateBall();
    //draw the ball
    shape[ballY][ballX] = 1;
    lastMoveTime = now;
  }
}

void setLEDM(int row, int col, int v){
  if (row > 7 && row < 16){ // центральний ряд матриць
    if (col < 8 && col >= 0){
      matrix.setLed(3, row - 8, col, v);
    } 
    else if (col >= 8 && col < 16){
      matrix.setLed(2, row - 8, col - 8, v);
    } 
    else if (col >= 16 && col < 24){
      matrix.setLed(1, row - 8, col - 16, v);
    }
  }
  else if (row > 15){ // нижня матриця
    matrix.setLed(0, row - 16, col - 8, v);
  }
  else { // верхня матриця
    matrix.setLed(4, row, col - 8, v);
  }  
}

void draw(){
  if(now - lastRefreshTime >= refreshInterval){
    for(int i = 0; i < 24; i++){
      for (int j = 0; j < 8; j++){
        setLEDM(i, j+8, shape[i][j]==1 ? HIGH:LOW); 
      }
    }
    
    currentColumn++;
    currentColumn %= 8;
    lastRefreshTime = now; 
  }
}

void updateScore(){
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 24; j++){
      shape[i][j] = 0;
    }
  }
  
  for(int i = 0; i < player2Score; i++){
    for(int j = 0; j < i + 1; j++){
      shape[j][6 - i] = 1;
    }
  }
  
  for(int i = 0; i < player1Score; i++){
    for(int j = 0; j < i + 1; j++){
      shape[7 - j][i + 1] = 1;
    }
  }
  if(now - overTime > 3000)
    restartGame();
}
