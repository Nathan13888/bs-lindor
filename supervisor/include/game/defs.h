//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_DEFS_H
#define SUPERVISOR_DEFS_H

#include <map>
#include <iostream>
#include <vector>

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

std::ostream& operator<<(std::ostream& o, Direction c);


const std::vector<Direction> DIRECTIONS = {Direction::North, Direction::South, Direction::West, Direction::East};
const int MAX_HEALTH = 100;
const int HEALTH_LOSS = 1;
const int FREE_MOVES = 2;

const std::map<CellType, std::string> CELL_STR_MAP {
        {CellType::empty, "  "},
        {CellType::wall, "X "},
        {CellType::food, "f "}
};

const std::map<Direction, std::string> DIR_STR_MAP {
        {Direction::North, "up"},
        {Direction::South, "down"},
        {Direction::West, "left"},
        {Direction::East, "right"}
};

#endif //SUPERVISOR_DEFS_H
