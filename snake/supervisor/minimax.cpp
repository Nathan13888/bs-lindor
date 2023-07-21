#pragma once

#include <ctime>
#include <math.h>
#include "../game/defs.cpp"
#include "../game/gameState.cpp"

using namespace std;

const int MAX_TIME = 200;

class Minimax {
    public:
        clock_t start;
        int depth_limit;
        double food_w;
        double length_w;
        double space_w;
        double alive_w;
        
        Minimax();
        Minimax(double food_weight, double length_weight, double space_weigth, double alive_weight);
        double scoreState(GameState gState, int idx);
        pair<double, Direction> alphabeta(GameState gState, snake_index idx, snake_index curr_idx, double alpha, double beta, int depth, int max_depth, Direction move);
        Direction makeMove(GameState gState, snake_index idx);
};

Minimax::Minimax() {}

Minimax::Minimax(double food_weight, double length_weight, double space_weight, double alive_weight) {
    this->food_w = food_weight;
    this->length_w = length_weight;
    this->space_w = space_weight;
    this->alive_w = alive_weight;
}

double Minimax::scoreState(GameState gState, int idx) {
    double score = 0;
    Snake snake = gState.getSnake(idx);


    // check end game
    if (!snake.isAlive()) {
        return numeric_limits<double>::lowest();
    }

    if (gState.numAlive() == 1) {
        return numeric_limits<double>::max();
    }

    // check space
    pair<int, int> p = gState.voronoi(idx);
    int space = p.first;
    int owned_food = p.second;

    if (!space) {
        return numeric_limits<double>::lowest();
    }

    score += space * space_w;

    // check remaining snakes
    score += numeric_limits<double>::max() / gState.numAlive() * alive_w;

    // check proximity to food
    Point head = snake.getHead();
    vector<Path> paths = gState.bfsFood(head);
    if (paths.size()) {
        int food_length = owned_food;
        if (owned_food = -1) {
            food_length = paths[0].length();
        }

        int life_line = snake.getHealth() - food_length;

        if (life_line < 0) {
            return numeric_limits<double>::lowest();
        }

        score += atan(life_line) * food_w;
    }

    // check snake length
    score += 1.0 / snake.getSize() * length_w;
}

pair<double, Direction> Minimax::alphabeta(GameState gState, snake_index idx, snake_index curr_idx, double alpha, double beta, int depth, int max_depth, Direction move_i) {
    Snake snake = gState.getSnake(idx);
    clock_t delta = (clock() - start) / (CLOCKS_PER_SEC / 1000);
    if (delta > MAX_TIME || !snake.isAlive() || depth > max_depth) {
        return make_pair(scoreState(gState, idx), move_i);
    }

    int next_idx = (curr_idx + 1) % gState.getSnakes().size();

    if (curr_idx == idx) {
        double curr_max = numeric_limits<double>::max();
        Direction curr_move;

        vector<Direction> our_moves = snake.getMoves();
        for (auto move : our_moves) {
            GameState nState = gState;
            nState.makeMove(move, idx);

            Snake nSnake = nState.getSnake(idx);
            Point nHead = nSnake.getHead();

            double scoreAdj = 0;

            // check for head on collision
            vector<Snake> snakes = nState.getSnakes();
            for (int i = 0; i < snakes.size(); i++) {
                Snake other_snake = snakes[i];
                if (i != idx && other_snake.isAlive()) {
                    vector<Point> neighbours = nState.getBoard().expand(other_snake.getHead());
                    for (Point neighbour : neighbours) {
                        if(neighbour == nHead && nSnake.getSize() <= other_snake.getSize()) {
                            scoreAdj += std::numeric_limits<double>::lowest();
                            break;
                        }
                    }
                }

            }

            // check whether a move is on a snake's tail where that snake can eat
            unordered_set<snake_index> occupants = gState.getBoard().getCellOccupants(nHead);
            if (occupants.size() > 0) {
                snake_index other_idx = *occupants.begin();
                Snake other_snake = gState.getSnake(other_idx);
                deque<Point> snake_points = other_snake.getPoints();
                Point p1 = snake_points.back();
                snake_points.pop_back();
                Point p2 = snake_points.back();
                if (p1 == p2) {
                    scoreAdj += std::numeric_limits<double>::lowest();
                }
            }

            pair<double, Direction> value = alphabeta(nState, idx, next_idx, alpha, beta, depth + 1, max_depth, move);
            if(scoreAdj < 0) {
                value.first = scoreAdj;
            }

            if (value.first > curr_max) {
                curr_max = value.first;
                curr_move = move;
            }

            if (curr_max > alpha) {
                alpha = curr_max;
            }

            if (beta <= alpha) {
                break;
            }
        }

        return make_pair(curr_max, curr_move);
    } else {
        double curr_min = numeric_limits<double>::max();
        Direction curr_move;

        Snake opp_snake = gState.getSnake(curr_idx);
        vector<Direction> opp_moves = opp_snake.getMoves();
        for (auto move : opp_moves) {
            GameState nState = gState;
            nState.makeMove(move, curr_idx);
            nState.cleanup();

            pair<double, Direction> value = alphabeta(nState, idx, next_idx, alpha, beta, depth + 1, max_depth, move);
            if (value.first < curr_min) {
                curr_min = value.first;
                curr_move = move;
            }

            if (curr_min < beta) {
                beta = curr_min;
            }

            if (beta <= alpha) {
                break;
            }
        }

        return make_pair(curr_min, curr_move);
    }
}

Direction Minimax::makeMove(GameState gState, snake_index idx) {
    start = clock();
    double alpha = numeric_limits<double>::lowest();
    double beta = numeric_limits<double>::max();
    pair<double, Direction> move_pair = alphabeta(gState, idx, idx, alpha, beta, 0, depth_limit, Direction::North);

    return move_pair.second;
}