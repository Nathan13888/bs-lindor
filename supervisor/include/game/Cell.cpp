//
// Created by ethan on 7/24/2023.
//
#include <cassert>

#include "Cell.h"

Cell::Cell(){
    type = CellType::empty;
}

void Cell::occupy(snake_index idx){
    occupants.insert(idx);
}

void Cell::vacate(snake_index idx){
    occupants.erase(idx);
}

void Cell::vacateAll(){
    occupants.clear();
}

size_t Cell::numOccupants(){
    return occupants.size();
}

unordered_set<snake_index> Cell::getOccupants(){
    return occupants;
}

CellType Cell::getType() const {
    return type;
}

void Cell::setType(CellType cellType){
    this->type = cellType;
}

bool Cell::isOccupant(snake_index idx){
    return occupants.find(idx) != occupants.end();
}

void Cell::setFood(){
    assert(type == CellType::empty);
    assert(numOccupants() == 0);
    type = CellType::food;
}