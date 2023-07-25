//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_DEFS_H
#define SUPERVISOR_DEFS_H

#include <map>
#include <iostream>
#include <vector>

using namespace std;

typedef int snake_index;

enum class CellType {
    empty,
    wall,
    food
};

enum class Direction {
    North = 2,
    South = 3,
    West = 0,
    East = 1
};

ostream& operator<<(std::ostream& o, Direction c);


const vector<Direction> DIRECTIONS = {Direction::North, Direction::South, Direction::West, Direction::East};
const int MAX_HEALTH = 100;
const int HEALTH_LOSS = 1;
const int FREE_MOVES = 2;

const map<CellType, string> CELL_STR_MAP {
        {CellType::empty, "  "},
        {CellType::wall, "X "},
        {CellType::food, "f "}
};

const map<Direction, string> DIR_STR_MAP {
        {Direction::North, "up"},
        {Direction::South, "down"},
        {Direction::West, "left"},
        {Direction::East, "right"}
};

#endif //SUPERVISOR_DEFS_H
