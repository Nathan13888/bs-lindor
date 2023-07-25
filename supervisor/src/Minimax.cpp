//
// Created by ethan on 7/24/2023.
//
#include <limits>
#include <cmath>
#include <utility>

#include "Minimax.h"

using namespace std;

Minimax::Minimax() {}

Minimax::Minimax(double food_weight, double length_weight, double space_weight, double alive_weight) {
    this->food_w = food_weight;
    this->length_w = length_weight;
    this->space_w = space_weight;
    this->alive_w = alive_weight;
}

double Minimax::scoreState(GameState gState, snake_index idx) const {
    double score = 0;
    Snake snake = gState.getSnake(idx);

    // check end game
    if (!snake.isAlive()) {
        return numeric_limits<double>::lowest();
    }

    if (gState.numAlive() == 1) {
        score = 10000;
    }

    score -= gState.getTick() * turns_w;

    // check space
    pair<int, int> p = gState.voronoi(idx);
    int space = p.first;
    int owned_food = p.second;

    if (!space) {
        return numeric_limits<double>::lowest();
    }

    score += space * space_w;

    // check remaining snakes
    score += 100000.0 / gState.numAlive();

    // check proximity to food
    Point head = snake.getHead();
    vector<Path> paths = gState.bfsFood(head);
    if (!paths.empty()) {
        int food_length = owned_food;
        if (owned_food == -1) {
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

    return score;
}

pair<double, Direction> Minimax::alphaBeta(GameState gState, snake_index idx, double alpha, double beta, int depth, int max_depth, Direction move_i, bool isMax) {
    Snake snake = gState.getSnake(idx);
    clock_t delta = (clock() - start) / (CLOCKS_PER_SEC / 1000);

    if (delta > MAX_TIME || !snake.isAlive() || depth > max_depth || gState.numAlive() == 1) {
        return make_pair(scoreState(gState, idx), move_i);
    }

    gState.cleanup();

    Board board = gState.getBoard();
    snake_index opp_idx = gState.getOpponent(idx);
    Snake opp_snake = gState.getSnake(opp_idx);

    //int next_idx = (curr_idx + 1) % gState.getSnakes().size();

    cout << "MAP: " << endl;
    gState.getBoard().print();
    cout << endl;

    cout << "depth: " << depth << endl;

    if (isMax) {
        double curr_max = numeric_limits<double>::min();
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
            if (!occupants.empty()) {
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

            pair<double, Direction> value = alphaBeta(nState, idx, alpha, beta, depth + 1, max_depth, move, false);

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

        vector<Direction> opp_moves = opp_snake.getMoves();
        for (auto move : opp_moves) {
            GameState nState = gState;
            if (opp_idx != idx) {
                nState.makeMove(move, opp_idx);
            }

            pair<double, Direction> value = alphaBeta(nState, idx, alpha, beta, depth + 1, max_depth, move, true);
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
    pair<double, Direction> move_pair = alphaBeta(std::move(gState), idx, alpha, beta, 0, depth_limit, Direction::East, true);

    cout << double(clock() - start) / double(CLOCKS_PER_SEC);

    return move_pair.second;
}