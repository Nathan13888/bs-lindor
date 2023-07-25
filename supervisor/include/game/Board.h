//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_BOARD_H
#define SUPERVISOR_BOARD_H

#include "Point.h"
#include "Cell.h"
#include "Path.h"

class Board {
    public:
        vector<vector<Cell>> board;
        int width;
        int height;
        Board();
        Board(int width, int height);
        void clear();
        void print();
        Point getRandomEmptyPoint();
        void occupyCell(Point p, snake_index idx);
        void vacateCell(Point p, snake_index idx);
        size_t cellNumOccupants(Point p);
        CellType getCellType(Point p);
        void setCellType(Point p, CellType type);
        unordered_set<snake_index> getCellOccupants(Point p);
        bool isOccupantOf(Point p, snake_index idx);
        vector<Point> getPoints();
        bool isValid();
        void placeFood(Point p);
        size_t getWidth();
        size_t getHeight();
        bool isSafe(Point p);
        bool in(Point p);
        vector<Point> expand(Point p);
        vector<Path> bfsFood(Point start);
        int floodFill(Point start);
};


#endif //SUPERVISOR_BOARD_H
