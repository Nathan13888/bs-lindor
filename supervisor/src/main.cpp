#include <iostream>
#include <string>

#include "Minimax.h"
#include "utils.h"

int main() {
    std::string input;
    Minimax treeSearch = Minimax();

    getline(std::cin, input);

    std::pair<GameState, snake_index> info = parseGameState(input);
    GameState game_state = info.first;
    snake_index idx = info.second;

    //game_state.getBoard().print();

    std::vector<std::pair<double, Direction>> moves = treeSearch.getMoves(game_state, idx);

    std::cout << encodeResponse(moves);

    return 0;
}
