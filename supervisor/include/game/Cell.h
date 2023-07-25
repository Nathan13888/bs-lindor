//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_CELL_H
#define SUPERVISOR_CELL_H

#include <unordered_set>

#include "defs.h"

using namespace std;

class Cell {
    private:
        unordered_set<snake_index> occupants;
        CellType type;
    public:
        Cell();
        void occupy(snake_index idx);
        void vacate(snake_index);
        void vacateAll();
        size_t numOccupants();
        unordered_set<snake_index> getOccupants();
        CellType getType() const;
        void setType(CellType cellType);
        bool isOccupant(snake_index idx);
        void setFood();
};


#endif //SUPERVISOR_CELL_H
