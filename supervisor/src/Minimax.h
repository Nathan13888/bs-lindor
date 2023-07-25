//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_MINIMAX_H
#define SUPERVISOR_MINIMAX_H

#include <ctime>
#include "../include/game/GameState.h"

const int MAX_TIME = 400;

class Minimax {
    public:
        clock_t start;
        int depth_limit = 7;
        double food_w = 68.60914;
        double length_w = 0.82267;
        double space_w = 7.60983;
        double alive_w = 1000;
        double turns_w = 5;

        Minimax();
        Minimax(double food_weight, double length_weight, double space_weigth, double alive_weight);
        double scoreState(GameState gState, int idx) const;
        pair<double, Direction> alphaBeta(GameState gState, snake_index idx, double alpha, double beta, int depth, int max_depth, Direction move, bool isMax);
        Direction makeMove(GameState gState, snake_index idx);
};


#endif //SUPERVISOR_MINIMAX_H
