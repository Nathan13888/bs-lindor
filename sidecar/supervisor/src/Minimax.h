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
        int depth_limit = 6;
        double food_w = 68.60914;
        double length_w = 0.82267;
        double space_w = 7.60983;
        double alive_w = 10000;
        double depth_w = -1;

        Minimax();
        Minimax(double food_weight, double length_weight, double space_weigth, double alive_weight, double depth_weight);
        [[nodiscard]] double scoreState(GameState game_state, int idx) const;
        double treeSearch(GameState game_state, snake_index curr_idx, snake_index our_idx, int depth);
        std::vector<std::pair<double, Direction>> getMoves(GameState game_state, snake_index idx);
};


#endif //SUPERVISOR_MINIMAX_H
