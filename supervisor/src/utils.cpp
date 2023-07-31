//
// Created by ethan on 7/24/2023.
//

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

#include "utils.h"

using JSON = nlohmann::json;

std::string encodeMove(Direction move) {
    JSON encodedMove;
    encodedMove["move"] = DIR_STR_MAP.at(move);
    encodedMove["shout"] = "I'm the supervisor :)";
    return encodedMove.dump();
}

std::string encodeResponse(const std::vector<std::pair<double, Direction>>& moves) {
    JSON encodedResponse;
    encodedResponse["moves"] = JSON::array();
    for (auto move : moves) {
        JSON encodedMove;
        encodedMove["move"] = DIR_STR_MAP.at(move.second);
        encodedMove["value"] = move.first;
        encodedResponse["moves"].push_back(encodedMove);
    }

    return encodedResponse.dump();
}

Point parsePoint(JSON encodedPoint) {
    int x = encodedPoint["x"];
    int y = encodedPoint["y"];
    return {x + 1, y + 1};
}

Snake parseSnake(JSON encodedSnake, int turn) {
    int free_moves = FREE_MOVES - turn;
    if (free_moves < 0) {
        free_moves = 0;
    }
    int health = encodedSnake["health"];
    std::string id = encodedSnake["id"];
    Snake snake = Snake(health, turn, free_moves, id);

    for (const auto& point : encodedSnake["body"]) {
        Point p = parsePoint(point);
        snake.addPoint(p);
    }

    return snake;
}

std::pair<GameState, snake_index> parseGameState(const std::string& body) {
    JSON j = JSON::parse(body);

    JSON board = j["board"];

    int height = board["height"];
    int width = board["width"];
    int turn = j["turn"];
    GameState gs = GameState(height, width);

    for (const auto& food_json : board["food"]) {
        Point p = parsePoint(food_json);
        gs.addFood(p);
    }

    std::string id = j["you"]["id"];
    snake_index me = -1;
    for (const auto& snake_json : board["snakes"]) {
        Snake snake = parseSnake(snake_json, turn);
        snake_index cur = gs.addSnake(snake);
        if (snake.getID() == id) {
            me = cur;
        }
    }

    assert(me != -1);

    return std::make_pair(gs, me);
}