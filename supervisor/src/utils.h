//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_UTILS_H
#define SUPERVISOR_UTILS_H

#include <string>
#include "../include/game/defs.h"
#include "../include/game/GameState.h"

#include <nlohmann/json.hpp>

using JSON = nlohmann::json;

std::string encodeMove(Direction move);

std::string encodeResponse(const std::vector<std::pair<double, Direction>>& moves);

Point parsePoint(JSON encodedPoint);

Snake parseSnake(JSON encodedSnake, int turn);

std::pair<GameState, snake_index> parseGameState(const std::string& body);

#endif //SUPERVISOR_UTILS_H
