# Проект "Retro Games Cube"

Проект "Retro Games Cube" студентів групи КН-115 кафедри СШІ НУ "Львівська Політехніка"

<img src="https://github.com/DanyloMelnyk/RetroGames/blob/master/img/model1.png" width="300">
<img src="https://github.com/DanyloMelnyk/RetroGames/blob/master/img/photo1.png" width="300">
<img src="https://github.com/DanyloMelnyk/RetroGames/blob/master/img/on.jpg" width="300">

## Структура файлів
* [Snake.ino](https://github.com/DanyloMelnyk/RetroGames/blob/master/Games/Snake/Snake.ino) - змійка для 1 гравця та 3 матриць;
* [SnakeFULL.ino](https://github.com/DanyloMelnyk/RetroGames/blob/master/Games/SnakeFULL/SnakeFULL.ino) - змійка для 1 гравця та 5 матриць;
* [Snake2player.ino](https://github.com/DanyloMelnyk/RetroGames/blob/master/Games/Snake2player/Snake2player.ino) - змійка для 2 гравців та 3 матриць;
* [Snake2playerFULL.ino](https://github.com/DanyloMelnyk/RetroGames/blob/master/Games/Snake2playerFULL/Snake2playerFULL.ino) - змійка для 2 гравців та 5 матриць;
* [pong.ino](https://github.com/DanyloMelnyk/RetroGames/blob/master/Games/pong/pong.ino) - ping-pong для 5 матриць;
* [Menu.ino](https://github.com/DanyloMelnyk/RetroGames/blob/master/Menu/Menu.ino) - ping-pong чи змійка для 2 гравців, меню вибору ігор при запуску arduino;
* [lib.ino](https://github.com/DanyloMelnyk/RetroGames/blob/master/lib/lib.ino) - ping-pong, змійка для 1 чи 2 гравців та меню, реалізовано з допомогою бібліотеки [Retrogame.h](https://github.com/DanyloMelnyk/RetroGames/blob/master/lib/Retrogame.h);
* [Retrogame.h](https://github.com/DanyloMelnyk/RetroGames/blob/master/lib/Retrogame.h) та [Retrogame.cpp](https://github.com/DanyloMelnyk/RetroGames/blob/master/lib/Retrogame.cpp) - бібліотека для обробки джойстиків, роботи з матрицями та виводу повідомлень;
* [template.ino](https://github.com/DanyloMelnyk/RetroGames/blob/master/template/template.ino) - шаблон для написання програм з коментарями (реалізовано для 5 матриць);
* [GameBoard.png](https://github.com/DanyloMelnyk/RetroGames/blob/master/GameBoard.png) - схема ігрового поля + адреси матриць.

## Автори

* **Мельник Данило** - *розробник програмного та апаратного забезпечення* - [GIT](https://github.com/DanyloMelnyk)
* **Пасемко Андрій** - *презентатор, розробник апаратного забезпечення* - [GIT](https://github.com/Pasemko)
* **Рокицька Анастасія, Галік Вікторія** - *дизайн, ідеї* - [GIT](https://github.com/Anastasiia-Rokytska), [GIT](https://github.com/VictoriaHalik)
* **Сирватка Максим** - *розробник програмного забезпечення, тестер* - [GIT](https://github.com/maxymsyrvatka)
* **Попів Христина** - *презентатор, тестер* - [GIT](https://github.com/Khrystynapopiv)



Список дописувачів [тут](https://github.com/DanyloMelnyk/RetroGames/graphs/contributors).

## TODO

- [X] Корпус для матриць і електроніки
- [ ] Корпус для джойстиків
- [X] Обробка натискання кнопок на джойстиках
- [X] Меню вибору ігор
- [ ] Додаткові ігри
- [ ] Фінальна пайка
- [ ] Презентація

- [ ] Перехід на STM32

## Список використаних джерел
* [LedControl](https://github.com/wayoda/LedControl/) - бібліотека для роботи з MAX-7219;
* [arduino-snake](https://github.com/ondt/arduino-snake/) - базовий алгоритм роботи гри “Snake”для однієї матриці;
* [LedMatrixPong](https://github.com/monur/LedMatrixPong/) - базовий алгоритм роботи гри "Pong”для однієї матриці.

#### Використані програми
* [Arduino IDE](https://www.arduino.cc/en/main/software "Arduino IDE") - середовище розробки, компілятор;
* [Visual Studio](https://visualstudio.microsoft.com/ "Visual Studio") - середовище розробки
  * [vMicro](https://www.visualmicro.com/ "vMicro") - плагін для Visual Studio для підтримки Arduino;
* [Autodesk Fusion 360](https://www.autodesk.com/products/fusion-360/overview "Autodesk Fusion 360") - 3D моделювання.
