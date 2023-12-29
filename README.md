# Fantasy Football

## Table of Contents

1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Usage](#usage)
4. [Built With](#built-with)
5. [Implementation](#implementation)
    1. [Exhaustive search algorithm](#exhaustive-search-algorithm)
    2. [Greedy algorithm](#greedy-algorithm)
    3. [Metaheuristic algorithm](#metaheuristic-algorithm)
6. [Authors](#authors)


## Introduction

The Fantasy Football project is a software application designed to assist users in managing their fantasy football teams. Given a database of football players, from LaLiga EA SPORTS 2022/2023 season, and a file with some restrictions: The number of defenses required `N1`, the number of midfielders required `N2`, the number of forwards required `N3`, the maximum price per player `max_price_per_player`, the maximum price for the whole team and the formation `max_cost`. The program will output the best possible team that can be created with the given players for every algorithm used. The program will also output the total cost of the team and the total points that the team is expected to score.


## Getting Started

To get started with this project, you will need to have the following prerequisites installed on your system:

1. C++ Compiler: Install a C++ compiler such as GCC or Clang on your system.

2. Operating System: Ensure that your operating system supports C++ development. Most modern operating systems, including Windows, macOS, and Linux, have built-in support for C++.

3. Libraries and Dependencies: Depending on your specific project requirements, you may need to install additional libraries or dependencies. Make sure to check the documentation or requirements of your project for any specific dependencies.

Once you have these prerequisites set up, you will be able to execute and compile .cc files in your workspace.


## Usage

There are two ways of running the program. The first one is by compiling and executing the files yourself. The second one is using the python files available in the repository, that make this process easier.

### Compiling and executing yourself

To check the output, use the following
```
g++ checker.cc -o checker
./checker <data_base.txt> <new_benchs/file.txt> <output.txt>
```
The result of this last command will be either "OK" or an error message.

### Using the python files
To run a single file, use:
```
python3 run_file.py <alg> <diff> <num> [nc]
```
Where `alg` is the algorithm to be used (exh, greedy or mh), `diff` is the difficulty of the problem (easy, med, hard), `num` is the number of the problem (from 1 to 7) and `nc` an optional argument that indicates if the program shold not compile the files, if the last argument is not nc, the program will compile the files.

To run multiple files at once, use the following command:
```
python3 run_all.py <alg> <diff> [nc]
```
Where alg is the algorithm to be used, diff is an opti"onal argument that indicates the difficulty of the problem, and nc an optional argument that indicates if the program shold not compile the files.

## Built With

Built using the C++11 standard programming language. Specifically using the C++11 standard.

## Implementation

We have considered three different algorithms to solve the problem. The first one is an exhaustive search algorithm, the second one is a greedy algorithm and the third one is a metaheuristic algorithm, more specifically a genetic algorithm.

Before explaining the algorithms, we will explain how we filtered the database of players. We don't select the players that follow : If the player exceeds the maximum cost per player, if the . Remove players that are worse in points and price than other players in the same position given the maximum number of players in each position  

### Exhaustive search algorithm

The exhaustive search algorithm is the simplest of the three. It consists of generating all the possible teams that can be created with the given players and then selecting the best one. This algorithm is very simple to implement and it will always find the best solution. 

### Greedy algorithm

The greedy algorithm is a bit more complex than the exhaustive search algorithm. It consists of sorting the vector of players by their points per cost ratio multiplied by the points up to some alpha and then selecting the best players that can be added to the team. When we only need one player to complete the team, we select the player with more points that can be added. This algorithm ends instantly but it will not always find the best solution.

### Metaheuristic algorithm

The metaheuristic algorithm is the most complex of the three. We have selected the genetic algorithm as our metaheuristic algorithm. The genetic algorithm consists of creating a population of teams, selecting the best teams, crossing them and mutating them. We have implemented two different ways of selecting the best teams, two different ways of crossing them and two different ways of mutating them. The first way of selecting the best teams is by selecting the best teams of the population. The second way of selecting the best teams is by selecting the best teams of the population and the best teams of the previous population. The first way of crossing the teams is by selecting a random number of players from the first team and the rest of the players from the second team. The second way of crossing the teams is by selecting a random number of players from the first team and the rest of the players from the second team, but the players that are selected from the first team are selected randomly. The first way of mutating the teams is by selecting a random number of players from the team and replacing them with random players. The second way of mutating the teams is by selecting a random number of players from the team and replacing them with random players, but the players that are selected from the team are selected randomly. This algorithm is the slowest of the three but it will always find a good solution.


## Authors

* **Gerard Grau** - *Initial work* - [gerardgrau](https://github.com/gerardgrau)
* **Pol Resina** - *Initial work* - [polresi](https://github.com/polresi)