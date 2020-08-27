# NE_project
This project reduces the problem of Nash-solvability in pure stationary strategies of the deterministic n-person games with perfect information and terminal payoffs to CNFSAT problem. The goal is to find an example where the cycle will be the worst outcome for all players and the game will not be Nash-solvable.  

A "bad" example was found with 4 players, the sum of the number of outcomes worse than cycle is 6 and the place of the cycle from the end is 3.

Player preferences:

* O1: a8 < a2 < c < a5 < a7 < a6 < a4 < a3
* O2: c < a6 < a3 < a2 < a8 < a7 < a5 < a4
* O3: a7 < a5 < c < a8 < a6 < a4 < a3 < a2
* O4: a6 < a8 < c < a7 < a5 < a4 < a3 < a2

Game graph:

![Image of game graph](https://github.com/ndchikin/NE_project/blob/master/graph.png)
## Install
Clone repository:
```bash
$ git clone https://github.com/ndchikin/NE_project.git
```
Install required packages and reboot:
```bash
$ cd NE_project
$ chmod 0777 install.sh
$ ./install.sh
$ sudo reboot
```
Build and install project:
```bash
$ mkdir build
$ cd build
$ cmake ../NE_project/
$ cmake --build .
$ cmake --install .
```
Then edit graphs.txt (optionaly), then run project:
```bash
$ ./start.sh
```
