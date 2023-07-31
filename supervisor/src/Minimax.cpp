//
// Created by ethan on 7/24/2023.
//
#include <limits>
#include <cmath>
#include <utility>

#include "Minimax.h"

Minimax::Minimax() {}

Minimax::Minimax(double food_weight, double length_weight, double space_weight, double alive_weight, double depth_weight) {
    this->food_w = food_weight;
    this->length_w = length_weight;
    this->space_w = space_weight;
    this->alive_w = alive_weight;
    this->depth_w = depth_weight;
}

double Minimax::scoreState(GameState game_state, snake_index idx) const {
    double score = 0;
    Snake snake = game_state.getSnake(idx);

    // check end game
    if (!snake.isAlive()) {
        return std::numeric_limits<double>::lowest();
    }

    if (game_state.numAlive() == 1) {
        score += 10000;
    }

    score += game_state.getTick() * depth_w;

    // check space
    std::pair<int, int> p = game_state.voronoi(idx);
    int space = p.first;
    int owned_food = p.second;

    if (!space) {
        return std::numeric_limits<double>::lowest();
    }

    score += space * space_w;

    // check remaining snakes
    score += alive_w / game_state.numAlive();

    // check proximity to food
    Point head = snake.getHead();
    std::vector<Path> paths = game_state.bfsFood(head);
    if (!paths.empty()) {
        int food_length = owned_food;
        if (owned_food == -1) {
            food_length = paths[0].length();
        }

        int life_line = snake.getHealth() - food_length;

        if (life_line < 0) {
            return std::numeric_limits<double>::lowest();
        }

        score += atan(life_line) * food_w;
    }

    // check snake length
    score += 1.0 / snake.getSize() * length_w;

    return score;
}

double Minimax::treeSearch(GameState game_state, snake_index curr_idx, snake_index our_idx, int depth) {
    Snake curr_snake = game_state.getSnake(curr_idx);
    Snake our_snake = game_state.getSnake(our_idx);

    if (depth >= depth_limit || !our_snake.isAlive() || game_state.numAlive() == 1) {
        return scoreState(game_state, our_idx);
    }

    while (!curr_snake.isAlive()) {
        curr_idx = (curr_idx + 1) % game_state.getSnakes().size();
        curr_snake = game_state.getSnake(curr_idx);
    }

    snake_index next_idx = (curr_idx + 1) % game_state.getSnakes().size();

    if (curr_idx == our_idx) {
        double curr_max = std::numeric_limits<double>::lowest();

        // search all possible moves
        std::vector<Direction> moves = our_snake.getMoves();
        for (Direction move : moves) {
            GameState new_state = game_state;
            new_state.makeMove(move, curr_idx);
            new_state.cleanup();
            double move_value = treeSearch(new_state, next_idx, our_idx, depth + 1);

            if (move_value > curr_max) {
                curr_max = move_value;
            }
        }

        return curr_max;
    } else {
        double curr_min = std::numeric_limits<double>::max();

        // search all possible moves
        std::vector<Direction> moves = curr_snake.getMoves();
        for (Direction move : moves) {
            GameState new_state = game_state;
            new_state.makeMove(move, curr_idx);
            new_state.cleanup();
            double move_value = treeSearch(new_state, next_idx, our_idx, depth + 1);

            if (move_value < curr_min) {
                curr_min = move_value;
            }
        }

        return curr_min;
    }
}

std::vector<std::pair<double, Direction>> Minimax::getMoves(GameState game_state, snake_index idx) {
    start = clock();
    std::vector<std::pair<double, Direction>> move_pairs;
    Snake our_snake = game_state.getSnake(idx);

    snake_index next_idx = idx;
    Snake next_snake = game_state.getSnake(next_idx);

    do {
        next_idx = (next_idx + 1) % game_state.getSnakes().size();
        next_snake = game_state.getSnake(next_idx);
    } while (!next_snake.isAlive());

    std::vector<Direction> moves = our_snake.getMoves();
    for (Direction move : moves) {
        GameState new_state = game_state;
        new_state.makeMove(move, idx);
        new_state.cleanup();

        std::pair<double, Direction> move_pair = std::make_pair(treeSearch(new_state, next_idx, idx, 0), move);
        move_pairs.push_back(move_pair);
    }

    //std::cout << "Time elapsed: " << double(clock() - start) / double(CLOCKS_PER_SEC) << "secs" << std::endl;

    return move_pairs;
}