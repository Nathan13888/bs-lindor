//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_GAMESTATE_H
#define SUPERVISOR_GAMESTATE_H

#include "Board.h"
#include "Snake.h"

const Point DEPTH_MARK = Point(-1, -1);

class GameState {
    private:
        int max_food;
        int cur_food;
        int tick;
        Board board;
        vector<Snake> snakes;
    public:
        GameState();
        GameState(int height, int width);
        GameState(int height, int width, int num_snakes);
        void makeMove(Direction dir, snake_index idx);
        snake_index addSnake();
        snake_index addSnake(Point start);
        snake_index addSnake(Snake snake);
        void addFood();
        void addFood(Point p);
        Snake& getSnake(snake_index idx);
        void removeFood(Point p);
        void checkCollision(Point cur_point);
        void cleanup();
        bool isValid();
        bool isGivenValid();
        Board getBoard();
        int getHeight();
        int getWidth();
        vector<Snake> getSnakes();
        void printScoreBoard();
        pair<int, int> voronoi(int voronoi_snake_id);
        int getTick();
        bool isSafe(Point p, int distance);
        bool willBeUnnocupied(Point p, int distance);
        vector<Path> bfsFood(Point start);
        snake_index getOpponent(snake_index idx);
        int floodFill(Point start);
        int numAlive();
    };


#endif //SUPERVISOR_GAMESTATE_H
