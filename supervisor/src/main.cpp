#include <iostream>
#include <string>

#include "Minimax.h"
#include "utils.h"

using namespace std;

int main() {
    string input;
    Minimax treeSearch = Minimax();

    cout << "Enter request body" << endl;
    getline(cin, input);

    pair<GameState, snake_index> info = parseGameState(input);
    GameState gState = info.first;
    snake_index idx = info.second;

    gState.getBoard().print();

    cout << "Determine move" << endl;

    Direction move = treeSearch.makeMove(gState, idx);

    cout << move << endl;

    string response = encodeMove(move);

    cout << response;

    return 0;
}
