//
// Created by ethan on 7/24/2023.
//
#include <cassert>

#include "Path.h"

void Path::add(Point p){
    path.push_front(p);
}

bool Path::exists(){
    return path.size() > 1;
}

Direction Path::getStepDir(int step) {
    assert(step >= 0 && step < path.size() - 1);
    Point one = path[step];
    Point two = path[step + 1];

    int x = two.x - one.x;
    int y = two.y - one.y;

    assert(x != 0 || y != 0);

    if (y == -1) {
        return Direction::North;
    } else if (y == 1) {
        return Direction::South;
    } else if (x == 1) {
        return Direction::East;
    }
    return Direction::West;
}

size_t Path::length(){
    return path.size() - 1;
}