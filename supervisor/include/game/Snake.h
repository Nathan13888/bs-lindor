//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_SNAKE_H
#define SUPERVISOR_SNAKE_H

#include <string>
#include <deque>
#include "Point.h"

using namespace std;

class Snake {
    private:
        bool alive;
        int health;
        int score;
        int free_moves;
        string id;
    public:
        deque<Point> points;
        Snake(Point start);
        Snake(Point start, int free_moves);
        Snake(int health, int score, int free_moves, string id);
        void setHealth(int new_health);
        int getHealth();
        int loseHealth();
        Point getHead();
        bool isAlive();
        void setAlive(bool alive);
        Point makeMove(Direction dir);
        Point popTail();
        size_t getSize();
        deque<Point> getPoints();
        void clearPoints();
        int getScore();
        int getFreeMoves();
        void useFreeMove();
        int getTurnsOccupied(Point p);
        vector<Direction> getMoves();
        void addPoint(Point p);
        bool inSnake(Point p);
        string getID();
};


#endif //SUPERVISOR_SNAKE_H
