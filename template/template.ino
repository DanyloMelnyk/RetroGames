#include "LedControl.h" // бібліотека для роботи з матрицями

#define MATRIX_NUM 5 // к-ксть матриць
#define ROW_NUM 24 // к-ксть рядків
#define COL_NUM 24 // к-ксть стовпців

struct Pin {
  static const short jX1 = A2;   // вісь X джойстика 1 гравця
  static const short jY1 = A3;   // вісь Y джойстика 1 гравця

  static const short jX2 = A0;   // вісь X джойстика 2 гравця
  static const short jY2 = A1;   // вісь Y джойстика 1 гравця

  static const short potentiometer = A7; // змінний резистор (регулятор швидкості і т.д.)

  static const short CLK = 10;   // CLK матриці
  static const short CS  = 11;  // CS матриці
  static const short DIN = 12; // DIN матрицці
};

const short intensity = 3; // яскравість(0(мін)-15(макс))
const short messageSpeed = 5; // швидкість прокрутки тексту

void setup() {
  Serial.begin(115200);  // ініціалізація COM інтерфейсу для виводу інфи на ПК
  initialize();         // ініціалізація матриць
  calibrateJoystick(); // визначення поч. положення джойстиків

  // тут код, що викон. 1 раз при завантаженні Arduino
}

void loop() {
  scanJoystick(); // обробка джойстика
  
  // тут код, що викон. у безкінечному циклі
}

LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, MATRIX_NUM); // Визначення класу для роботи з матрицями

struct Point { // оголош типу даних Point
  int row = 0, col = 0;
  Point(int row = 0, int col = 0): row(row), col(col) {}
};

struct Coordinate { // оголош типу даних Coordinate
  int x = 0, y = 0;
  Coordinate(int x = 0, int y = 0): x(x), y(y) {}
};

void setLEDM(int row, int col, int v){ //встановлення стану діода(пікселя) на рядку row(нумерація зверху) і стовпці row(нум. зліва) на v(1-включений, 0 - викл)
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

int Dir1 = 0; // напрямок 1 гравця (0-не рух, up, down, right, left)
int Dir2 = 0; // напрямок 2 гравця (0-не рух, up, down, right, left)

const short up     = 1; // конст. напрямку
const short right  = 2;
const short down   = 3;
const short left   = 4;

const int joystickThreshold = 160; // Поріг, до якого переміщення джойстика ігноруються

Coordinate jHome1(500, 500); // змінна для поч. положення джойстиків
Coordinate jHome2(500, 500);

int Speed = 1;

void scanJoystick() { // визнач напрямку
  int pDir1 = Dir1; // збереження минулого напрямку
  int pDir2 = Dir2;
  
  long timestamp = millis(); // Засікти час

  while (millis() < timestamp + Speed) { // коли пройшло (Speed) мс
    float raw = mapf(analogRead(Pin::potentiometer), 0, 1023, 0, 1); // отримання полож. змінного резистора (від 0.0 до 1.0)

    analogRead(Pin::jY1) < jHome1.y - joystickThreshold ? Dir1 = down : 0; // зчитування 1 джойстика
    analogRead(Pin::jY1) > jHome1.y + joystickThreshold ? Dir1 = up : 0; //down  : 0;
    analogRead(Pin::jX1) < jHome1.x - joystickThreshold ? Dir1 = left  : 0;
    analogRead(Pin::jX1) > jHome1.x + joystickThreshold ? Dir1 = right : 0;

    Dir1 + 2 == pDir1 && pDir1 != 0 ? Dir1 = pDir1 : 0; // заборона зміни напрямку на протилеж
    Dir1 - 2 == pDir1 && pDir1 != 0 ? Dir1 = pDir1 : 0;

    analogRead(Pin::jY2) < jHome2.y - joystickThreshold ? Dir2 = down : 0; // зчитування 2 джойстика
    analogRead(Pin::jY2) > jHome2.y + joystickThreshold ? Dir2 = up : 0;
    analogRead(Pin::jX2) < jHome2.x - joystickThreshold ? Dir2 = left  : 0;
    analogRead(Pin::jX2) > jHome2.x + joystickThreshold ? Dir2 = right : 0;

    Dir2 + 2 == pDir2 && pDir2 != 0 ? Dir2 = pDir2 : 0; // заборона зміни напрямку на протилеж
    Dir2 - 2 == pDir2 && pDir2 != 0 ? Dir2 = pDir2 : 0;
  }
}

void calibrateJoystick() { // визначення поч. положення джойстиків
  Coordinate values;

  for (int i = 0; i < 10; i++) {
    values.x += analogRead(Pin::jX1);
    values.y += analogRead(Pin::jY1);
  }

  jHome1.x = values.x / 10;
  jHome1.y = values.y / 10;

  values.x = 0;
  values.y = 0;

  for (int i = 0; i < 10; i++) {
    values.x += analogRead(Pin::jX2);
    values.y += analogRead(Pin::jY2);
  }

  jHome2.x = values.x / 10;
  jHome2.y = values.y / 10;
}

void initialize() {  // ініціалізація матриць
  for (int i = 0; i < MATRIX_NUM; i++){
    matrix.shutdown(i, false);
    matrix.setIntensity(i, intensity);
    matrix.clearDisplay(i);
  }
  
  randomSeed(analogRead(A5));
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Danylo Melnyk KN115(AI) 11.10.2019
