//
// Created by ethan on 7/24/2023.
//

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

#include "utils.h"

using namespace std;
using JSON = nlohmann::json;

string encodeMove(Direction move) {
    JSON encodedMove;
    encodedMove["move"] = DIR_STR_MAP.at(move);
    encodedMove["shout"] = "I'm the supervisor :)";
    return encodedMove.dump();
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
    string id = encodedSnake["id"];
    Snake snake = Snake(health, turn, free_moves, id);

    for (const auto& point : encodedSnake["body"]) {
        Point p = parsePoint(point);
        snake.addPoint(p);
    }

    return snake;
}

pair<GameState, snake_index> parseGameState(const string& body) {
    cout << endl;
    cout << "Body: " << body << endl;

    JSON j = JSON::parse(body);

    JSON board = j["board"];
    cout << "Board: " << board << endl;

    int height = board["height"];
    int width = board["width"];
    int turn = j["turn"];
    GameState gs = GameState(height, width);

    for (const auto& food_json : board["food"]) {
        Point p = parsePoint(food_json);
        gs.addFood(p);
    }

    cout << "Updated Food" << endl;

    string id = j["you"]["id"];
    cout << id << endl;
    snake_index me = -1;
    for (const auto& snake_json : board["snakes"]) {
        Snake snake = parseSnake(snake_json, turn);
        snake_index cur = gs.addSnake(snake);
        if (snake.getID() == id) {
            me = cur;
        }
    }

    cout << "Updated Snakes" << endl;

    assert(me != -1);

    return make_pair(gs, me);
}