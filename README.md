# NE_project
This project reduces the problem of Nash-solvability in pure stationary strategies of the deterministic n-person games with perfect information and terminal payoffs to CNFSAT problem. The goal is to find an example where the cycle will be the worst outcome for all players and the game will not be Nash-solvable.
## Install
Clone repository:
```
$ git clone https://github.com/ndchikin/NE_project.git
```
Install required packages and reboot:
```
$ ./install.sh
$ sudo reboot
```
Make build directory:
```
$ mkdir build
$ cd build
```
Build and install project:
```
$ cmake ../NE_project/
$ cmake --build .
$ cmake --install .
```
Then edit graphs.txt (optionaly), then run project:
```
$ ./start.sh
```
