//
// Created by ethan on 7/24/2023.
//
#include <cassert>
#include <unordered_set>
#include <unordered_map>
#include <queue>

#include "Board.h"

Board::Board(){
}

Board::Board(int width, int height) {
    board.resize(height + 2);
    for(int i = 0; i < height + 2; i++){
        board[i].resize(width + 2);
    }
    clear();
}

void Board::clear(){
    for(auto y = board.begin(); y != board.end(); y++){
        for(auto x = y->begin(); x != y->end(); x++){
            if(x == y->begin() || x == y->end() - 1 || y == board.begin() || y == board.end() - 1){
                x->setType(CellType::wall);
                x->vacateAll();
            }else{
                x->setType(CellType::empty);
                x->vacateAll();
            }
        }
    }
}

void Board::print(){
    for (const auto& row : board) {
        for (auto cell : row) {
            if(cell.numOccupants() > 0){
                for(auto occupant : cell.getOccupants()){
                    std::cout << occupant;
                }
                std::cout << ' ';
            }else{
                std::cout << CELL_STR_MAP.at(cell.getType());
            }
        }
        std::cout << std::endl;
    }
}

Point Board::getRandomEmptyPoint(){
    int x;
    int y;
    do{
        x = rand() % board.size();
        y = rand() % board[x].size();
    }while(board[y][x].getType() != CellType::empty || board[y][x].numOccupants() != 0);
    return {x, y};
}

void Board::occupyCell(Point p, snake_index idx){
    board[p.y][p.x].occupy(idx);
}

void Board::vacateCell(Point p, snake_index idx){
    board[p.y][p.x].vacate(idx);
}

size_t Board::cellNumOccupants(Point p){
    return board[p.y][p.x].numOccupants();
}

CellType Board::getCellType(Point p){
    return board[p.y][p.x].getType();
}

void Board::setCellType(Point p, CellType type){
    board[p.y][p.x].setType(type);
}

std::unordered_set<snake_index> Board::getCellOccupants(Point p){
    return board[p.y][p.x].getOccupants();
}

bool Board::isOccupantOf(Point p, snake_index idx){
    assert(in(p));
    return board[p.y][p.x].isOccupant(idx);
}

std::vector<Point> Board::getPoints(){
    std::vector<Point> points;
    for(auto y = board.begin(); y != board.end(); y++){
        for(auto x = y->begin(); x != y->end(); x++){
            Point p = Point(x - y->begin(), y - board.begin());
            points.push_back(p);
        }
    }
    return points;
}

bool Board::isValid(){
    for (const auto& row : board) {
        for (auto cell : row) {
            if(cell.numOccupants() > 1){
                std::cout << "Num occupants is greater than 1" << std::endl;
                return false;
            }
            if(cell.numOccupants() == 1 && cell.getType() != CellType::empty){
                std::cout << "Num occupants is 1 but celltype is not empty" << std::endl;
                return false;
            }
        }
    }
    return true;
}

void Board::placeFood(Point p){
    board[p.y][p.x].setFood();
}

size_t Board::getWidth() const{
    return board.size();
}

size_t Board::getHeight(){
    return board[0].size();
}

bool Board::isSafe(Point p){
    return board[p.y][p.x].getType() != CellType::wall && cellNumOccupants(p) == 0;
}

bool Board::in(Point p){
    return p.y >= 0 && p.y < board.size() && p.x >= 0 && p.x < board[p.y].size();
}

std::vector<Point> Board::expand(Point p) {
    std::vector<Point> neighbours = std::vector<Point>();
    for (auto d : DIRECTIONS) {
        Point n = p.addMove(d);
        if(in(n)){
            neighbours.push_back(n);
        }
    }
    return neighbours;
}

std::vector<Path> Board::bfsFood(Point start){
    std::vector<Path> paths;
    std::queue<Point> q = std::queue<Point>();
    std::unordered_set<Point> visited = std::unordered_set<Point>();
    std::unordered_map<Point, Point> parent = std::unordered_map<Point, Point>();
    visited.insert(start);
    q.push(start);
    while(!q.empty()){
        Point cur = q.front();
        q.pop();
        for(auto point: expand(cur)){
            if(in(point) && isSafe(point) && visited.find(point) == visited.end()){
                parent[point] = cur;
                visited.insert(point);
                if(getCellType(point) == CellType::food){
                    Path path = Path();
                    while(start != point){
                        path.add(point);
                        point = parent[point];
                    }
                    path.add(point);
                    paths.push_back(path);
                }
                q.push(point);
            }
        }
    }
    return paths;
}

int Board::floodFill(Point start){
    std::queue<Point> q = std::queue<Point>();
    std::unordered_set<Point> visited = std::unordered_set<Point>();
    visited.insert(start);
    q.push(start);
    while(!q.empty()){
        Point cur = q.front();
        q.pop();
        for(auto point: expand(cur)){
            if(in(point) && isSafe(point) && visited.find(point) == visited.end()){
                visited.insert(point);
                q.push(point);
            }
        }
    }
    return visited.size();
}