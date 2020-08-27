# NE_project
This project reduces the problem of Nash-solvability in pure stationary strategies of the deterministic n-person games with perfect information and terminal payoffs to CNFSAT problem. The goal is to find an example where the cycle will be the worst outcome for all players and the game will not be Nash-solvable.
Был найден «плохой» пример с 4 игроками, суммой числа исходов хуже цикла 6 и местом цикла с конца 3. 

Порядки предпочтения исходов игроками:

O1: a8 < a2 < c < a5 < a7 < a6 < a4 < a3
O2: c < a6 < a3 < a2 < a8 < a7 < a5 < a4
O3: a7 < a5 < c < a8 < a6 < a4 < a3 < a2
O4: a6 < a8 < c < a7 < a5 < a4 < a3 < a2

Граф игры:

![Image of game graph](https://github.com/ndchikin/NE_project/blob/master/graph.png)
## Install
Clone repository:
```
$ git clone https://github.com/ndchikin/NE_project.git
```
Install required packages and reboot:
```
$ cd NE_project
$ ./install.sh
$ sudo reboot
```
Build and install project:
```
$ mkdir build
$ cd build
$ cmake ../NE_project/
$ cmake --build .
$ cmake --install .
```
Then edit graphs.txt (optionaly), then run project:
```
$ ./start.sh
```
