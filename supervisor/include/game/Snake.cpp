//
// Created by ethan on 7/24/2023.
//
#include <cassert>
#include <algorithm>

#include "Snake.h"

Snake::Snake(Point start) {
    health = MAX_HEALTH;
    setAlive(true);
    score = 0;
    points.push_back(start);
    this->free_moves = FREE_MOVES;
}

Snake::Snake(Point start, int free_moves) {
    health = MAX_HEALTH;
    setAlive(true);
    score = 0;
    points.push_back(start);
    this->free_moves = free_moves;
}

Snake::Snake(int health, int score, int free_moves, std::string id) {
    this->health = health;
    this->score = score;
    this->free_moves = free_moves;
    this->id = id;
    setAlive(true);
}

Point Snake::getHead() {
    return *points.begin();
}

void Snake::setHealth(int new_health) {
    health = new_health;
}

int Snake::getHealth() {
    return health;
}

int Snake::loseHealth() {
    health -= HEALTH_LOSS;
    return health;
}

bool Snake::isAlive() {
    return alive;
}

void Snake::setAlive(bool alive) {
    this->alive = alive;
}

Point Snake::makeMove(Direction dir) {
    Point head = *points.begin();
    Point new_head = head.addMove(dir);
    points.push_front(new_head);
    score++;
    return new_head;
}

Point Snake::popTail() {
    Point back = points.back();
    points.pop_back();
    return back;
}


size_t Snake::getSize() {
    return points.size();
}

std::deque<Point> Snake::getPoints() {
    return points;
}

void Snake::clearPoints() {
    points.clear();
}

int Snake::getScore() {
    return score;
}

int Snake::getFreeMoves() {
    return free_moves;
}

void Snake::useFreeMove() {
    free_moves--;
}

int Snake::getTurnsOccupied(Point p) {
    int index = find(points.begin(), points.end(), p) - points.begin();
    assert(index != points.size());
    return points.size() - index;
}

std::vector<Direction> Snake::getMoves() {
    std::vector<Direction> moves = std::vector<Direction>();
    Point head = getHead();
    for (auto dir : DIRECTIONS) {
        Point p = head.addMove(dir);
        if (getSize() > 1) {
            if (points[1] != p) {
                moves.push_back(dir);
            }
        } else {
            moves.push_back(dir);
        }
    }
    return moves;
}

void Snake::addPoint(Point p) {
    points.push_back(p);
}

bool Snake::inSnake(Point p) {
    return find(points.begin(), points.end(), p) != points.end();
}

std::string Snake::getID() {
    return id;
}
